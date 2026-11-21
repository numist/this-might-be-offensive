<?php

require_once("offensive/assets/core.inc");
require_once("offensive/assets/contributions.inc");
require_once("offensive/assets/hof.inc");

// a pic page opened from a list other than the main feed (a user's
// contributions, the yearbook, the hall of fame) steps through that list instead: newer/older, the arrow
// keys and swiping all follow its links. the list is named in the URL
// (&ctx=...), so a link can be shared and back works, and nothing is kept
// in the session. browsing a list doesn't move the pickup link, which
// marks how far through the main feed the viewer has read, unless the
// list is the one the pickup link belongs to.
abstract class PicContext {

	// the list the request names, or null for the main feed. a list that
	// doesn't hold $upload is ignored.
	static function fromRequest($upload) {
		$ctx = isset($_REQUEST['ctx']) && is_string($_REQUEST['ctx']) ? $_REQUEST['ctx'] : "";
		if($ctx === "contributions") {
			return ContributionsContext::fromRequest($upload);
		}
		if($ctx === "yearbook") {
			return YearbookContext::fromRequest($upload);
		}
		if($ctx === "hof") {
			return HofContext::fromRequest($upload);
		}
		return null;
	}

	protected $upload;

	function __construct($upload) {
		$this->upload = $upload;
	}

	// what to append to a pic page link to keep browsing this list.
	abstract function query();

	// the list itself, for the index link.
	abstract function listLink();

	// "bob's contributions, by votes", as HTML.
	abstract function describe();

	// the neighbor shown above (before) or below (after) $upload in the
	// list, or false at either end.
	abstract protected function neighbor($before);

	// whether viewing an upload in this list moves the pickup link.
	function movesPickup() {
		return false;
	}

	// labels for the links to the item before and after. a list in date
	// order keeps the main feed's.
	function labels() {
		return array("newer", "older");
	}

	function before() {
		return $this->cached(true);
	}

	function after() {
		return $this->cached(false);
	}

	function link($upload) {
		return Link::upload($upload).$this->query();
	}

	// the upload's comments, still browsing this list, so the links
	// back to the pic page keep it.
	function threadLink($upload) {
		return Link::thread($upload).$this->query();
	}

	// query() as hidden form fields, for a form that redirects back.
	function hiddenFields() {
		parse_str(ltrim($this->query(), "&"), $params);
		$html = "";
		foreach($params as $name => $value) {
			$html .= '<input type="hidden" name="'.htmlEscape($name).'" value="'.htmlEscape($value).'"/>';
		}
		return $html;
	}

	private $neighbors = array();

	private function cached($before) {
		if(!array_key_exists((int)$before, $this->neighbors)) {
			$this->neighbors[(int)$before] = $this->neighbor($before);
		}
		return $this->neighbors[(int)$before];
	}

	// the main feed's filters (Upload::next_filtered()): squelched users
	// and, if the viewer hides them, nsfw and tmbo uploads.
	protected function filterSql() {
		$sql = me()->getPref("hide_tmbo") ? " AND NOT up.tmbo" : "";
		$sql .= me()->getPref("hide_nsfw") ? " AND NOT up.nsfw" : "";
		$squelched = array_map("intval", me()->squelched_list());
		if(count($squelched)) {
			$sql .= " AND up.userid NOT IN (".implode(",", $squelched).")";
		}
		return $sql;
	}

	// the first upload past $upload in the list's order, as keyset
	// pagination: $cols are the list's sort columns, all descending (or
	// all ascending, with $ascending), and $values $upload's values for
	// them. before means toward the top. $hints are index hints, as
	// table => hint, like core_getuploads() gives its lists.
	protected function keyset($where, $cols, $values, $before, $hints = array(), $ascending = false) {
		$up = $before != $ascending;
		$cmp = $up ? ">" : "<";
		$dir = $up ? "ASC" : "DESC";
		$order = array();
		foreach($cols as $col) {
			$order[] = "$col $dir";
		}
		// spelled out rather than as a row comparison, (a, b) < (x, y),
		// which MariaDB can't walk an index for: on the hall of fame
		// that's 1ms against 160ms for 400,000 uploads.
		$past = array();
		for($i = 0; $i < count($cols); $i++) {
			$terms = array();
			for($j = 0; $j < $i; $j++) {
				$terms[] = "{$cols[$j]} = {$values[$j]}";
			}
			$terms[] = "{$cols[$i]} $cmp {$values[$i]}";
			$past[] = "(".implode(" AND ", $terms).")";
		}
		$sql = uploadsql()." $where".$this->filterSql()."
		       AND (".implode(" OR ", $past).")
		       ORDER BY ".implode(", ", $order)."
		       LIMIT 1";
		foreach($hints as $table => $hint) {
			$sql = str_replace($table, "$table $hint", $sql);
		}
		$result = tmbo_query($sql);
		$row = mysqli_fetch_assoc($result);
		return $row ? new Upload($row) : false;
	}
}

// ?c=contributions&userid=<id>&sort=<sort>, in the same order. its
// discussions are skipped, since they have no pic page.
class ContributionsContext extends PicContext {
	private $user;
	private $sort;

	static function fromRequest($upload) {
		if(!isset($_REQUEST['userid']) || !is_intger($_REQUEST['userid'])) {
			return null;
		}
		$user = $upload->uploader();
		if($user->id() != $_REQUEST['userid'] || me()->squelched($user)) {
			return null;
		}
		return new ContributionsContext($upload, $user, contributionsSort());
	}

	function __construct($upload, $user, $sort) {
		parent::__construct($upload);
		$this->user = $user;
		$this->sort = $sort;
	}

	function query() {
		return contributionsContextQuery($this->user->id(), $this->sort);
	}

	function listLink() {
		return contributionsLink($this->user->id(), $this->sort);
	}

	function describe() {
		$name = $this->user->username();
		$orders = array("" => "newest first", "votes" => "by votes", "years" => "by votes within each year");
		return '<a href="'.$this->listLink().'">'.htmlEscape($name)."'".(substr($name, -1) == "s" ? "" : "s")
		       ." contributions</a>, ".$orders[$this->sort];
	}

	function labels() {
		return $this->sort === "" ? parent::labels() : array("prev", "next");
	}

	// contributionsQuery()'s orders. like it, this reads all of the user's
	// uploads from the userid index, which is cheap even for a prolific one.
	protected function neighbor($before) {
		$u = $this->upload;
		$id = (int)$u->id();
		$good = (int)$u->goods();
		// contributionsQuery() breaks ties in votes by the count cache's id
		$caid = "(SELECT id FROM offensive_count_cache WHERE threadid = $id)";
		if($this->sort == "votes") {
			$cols = array("ca.good", "ca.id");
			$values = array($good, $caid);
		} else if($this->sort == "years") {
			$cols = array("YEAR(up.timestamp)", "ca.good", "ca.id");
			$values = array((int)substr($u->timestamp(), 0, 4), $good, $caid);
		} else {
			$cols = array("up.timestamp", "up.id");
			$values = array("'".sqlEscape($u->timestamp())."'", $id);
		}
		$where = "AND up.userid = ".(int)$this->user->id()." AND up.type <> 'topic'";
		return $this->keyset($where, $cols, $values, $before, array("offensive_uploads up" => "USE KEY (userid)"));
	}
}

// ?c=yearbook&sort=<sort>: each user's newest yearbook photo from the
// last year, as core_getyearbook() lists them, a few hundred at most.
class YearbookContext extends PicContext {
	private $sort;

	// sort => array(column, ascending, description)
	private static function sorts() {
		return array(
			"uname_asc" => array("u.username", true, "by username"),
			"uname_desc" => array("u.username", false, "by username, z to a"),
			"date_desc" => array("up.id", false, "newest first"),
			"date_asc" => array("up.id", true, "oldest first"),
		);
	}

	static function fromRequest($upload) {
		if($upload->type() != "avatar") {
			return null;
		}
		$sort = isset($_REQUEST['sort']) && is_string($_REQUEST['sort']) ? $_REQUEST['sort'] : "";
		if(!array_key_exists($sort, self::sorts())) {
			return null;
		}
		return new YearbookContext($upload, $sort);
	}

	function __construct($upload, $sort) {
		parent::__construct($upload);
		$this->sort = $sort;
	}

	function query() {
		return yearbookContextQuery($this->sort);
	}

	function listLink() {
		return Link::content("yearbook")."&sort=".$this->sort;
	}

	function describe() {
		$sorts = self::sorts();
		return '<a href="'.$this->listLink().'">the yearbook</a>, '.$sorts[$this->sort][2];
	}

	// the yearbook's pickup link is the one for yearbook photos.
	function movesPickup() {
		return true;
	}

	function labels() {
		if($this->sort == "date_desc") return array("newer", "older");
		if($this->sort == "date_asc") return array("older", "newer");
		return array("prev", "next");
	}

	protected function neighbor($before) {
		$sorts = self::sorts();
		list($col, $ascending) = $sorts[$this->sort];
		$value = $col == "up.id"
		         ? (int)$this->upload->id()
		         : "'".sqlEscape($this->upload->uploader()->username())."'";
		$where = "AND up.type = 'avatar'
		          AND up.id = (SELECT MAX(newest.id) FROM offensive_uploads newest
		                       WHERE newest.type = 'avatar' AND newest.userid = up.userid)
		          AND u.account_status != 'locked'
		          AND up.timestamp > DATE_SUB(NOW(), INTERVAL 12 MONTH)";
		return $this->keyset($where, array($col), array($value), $before, array(), $ascending);
	}
}

// what a pic page link carries to browse the yearbook in $sort.
function yearbookContextQuery($sort) {
	return "&ctx=yearbook&sort=".rawurlencode($sort);
}

// the hall of fame's views: all time, the past 24 hours, the top 10 of
// each year, or one year (?c=hof, &t=today, &t=years, &y=<year>), in the
// same order. like the lists, it leaves out cellophane's images.
class HofContext extends PicContext {
	private $view;
	private $year;

	static function fromRequest($upload) {
		if($upload->type() != "image" || $upload->uploader()->id() == 1138) {
			return null;
		}
		$t = isset($_REQUEST['t']) && is_string($_REQUEST['t']) ? $_REQUEST['t'] : "";
		$year = false;
		if(isset($_REQUEST['y']) && is_intger($_REQUEST['y'])) {
			$year = (int)$_REQUEST['y'];
			$view = "year";
			if($year != (int)substr($upload->timestamp(), 0, 4)) {
				return null;
			}
		} else if($t === "today") {
			$view = "today";
			if(strtotime($upload->timestamp()) <= time() - 60 * 60 * 24) {
				return null;
			}
		} else if($t === "years") {
			$view = "years";
			if(self::yearRank($upload->id()) === false) {
				return null;
			}
		} else {
			$view = "hof";
		}
		return new HofContext($upload, $view, $year);
	}

	// where an upload is in the by year view's list, or false.
	private static function yearRank($id) {
		foreach(hofYearRanks(10) as $i => $rank) {
			if($rank[0] == $id) {
				return $i;
			}
		}
		return false;
	}

	function __construct($upload, $view, $year) {
		parent::__construct($upload);
		$this->view = $view;
		$this->year = $year;
	}

	function query() {
		return hofContextQuery($this->view, $this->year);
	}

	function listLink() {
		return Link::content("hof").substr($this->query(), strlen("&ctx=hof"));
	}

	function describe() {
		$what = array("hof" => "all time", "today" => "past 24 hours", "years" => "by year", "year" => (string)$this->year);
		return '<a href="'.$this->listLink().'">the hall of fame</a>, '.$what[$this->view];
	}

	function labels() {
		return array("prev", "next");
	}

	protected function neighbor($before) {
		$id = (int)$this->upload->id();
		$where = "AND up.type = 'image' AND up.userid <> 1138";

		if($this->view == "years") {
			// the next of the cached ranks that the viewer's filters let
			// through, in one query
			$ranks = hofYearRanks(10);
			$at = self::yearRank($id);
			$ids = array();
			foreach($before ? array_reverse(array_slice($ranks, 0, $at)) : array_slice($ranks, $at + 1) as $rank) {
				$ids[] = (int)$rank[0];
			}
			if(count($ids) == 0) {
				return false;
			}
			$list = implode(",", $ids);
			$result = tmbo_query(uploadsql()." $where".$this->filterSql()."
			                     AND up.id IN ($list) ORDER BY FIELD(up.id, $list) LIMIT 1");
			$row = mysqli_fetch_assoc($result);
			return $row ? new Upload($row) : false;
		}

		// core_getuploads()'s votes_desc, and its index hints
		$cols = array("ca.good", "ca.id");
		$values = array((int)$this->upload->goods(), "(SELECT id FROM offensive_count_cache WHERE threadid = $id)");
		if($this->view == "hof") {
			$hints = array("offensive_uploads up" => "USE KEY (status_type_id)",
			               "offensive_count_cache ca" => "USE KEY (good)");
		} else {
			// a day or a year: sorting its images is cheaper than walking
			// every image by votes to find the next one in it
			$hints = array("offensive_uploads up" => "USE KEY (t_t_id)");
			if($this->view == "today") {
				$where .= " AND up.timestamp > '".date("Y-m-d H:i:s", time() - 60 * 60 * 24)."'";
			} else {
				$where .= " AND up.timestamp > '".($this->year - 1)."-12-31 23:59:59'
				            AND up.timestamp < '".($this->year + 1)."-01-01 00:00:00'";
			}
		}
		$hints["users u"] = "USE KEY (PRIMARY)";
		return $this->keyset($where, $cols, $values, $before, $hints);
	}
}

// what a pic page link carries to browse a hall of fame view: "hof" (all
// time), "today", "years", or a year with $year.
function hofContextQuery($view, $year) {
	if($year) return "&ctx=hof&y=".(int)$year;
	if($view == "today" || $view == "years") return "&ctx=hof&t=$view";
	return "&ctx=hof";
}

?>