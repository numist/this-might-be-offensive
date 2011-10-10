<?

	$defaultpath = ini_get('include_path');
	set_include_path("..");
	require_once("offensive/assets/header.inc");

	time_start($ptime);

    require_once("offensive/assets/classes.inc");
    $me = false;

    mustLogIn();

    $me = new User(array(
        "userid" => $_SESSION["userid"],
        "username" => $_SESSION["username"],
        ));

	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/assets/tabs.inc' );
	require_once("offensive/assets/classes.inc");
	require_once("offensive/assets/core.inc");

    $numPerPage = 36;
    $args = $_GET;

    function randtld($username) {
        if($username == "numist") return ".net";
        if($username == "themaxx") return ".com";

        $tlds = array("net", "com", "com", "com", "com", "org", "biz", "com", "com", "com", "com", "com", "com", "com", "com", "com", "com", "org", "com", "com", "lu", "com", "com", "ar", "mx", "net", "com");
        return ".".$tlds[rand(0, count($tlds) - 1)];
    }

    function pickuplink() {
        // <!-- <a href="/offensive/?c=settings">Show options...</a> -->
    	// get db pickuplink
	    global $me, $activeTab;
	
		$prefname = "ipickup";
		$cookiename = $me->id()."lastpic";
	
	    // get db pickuplink
	    $dbpic = $me->getPref($prefname);

	    // get cookie pickuplink
	    if(array_key_exists($cookiename, $_COOKIE) && is_intger($_COOKIE[$cookiename])) {
		    $cookiepic = $_COOKIE[$cookiename];
	    } else {
		    $cookiepic = 0;
	    }
	
	    // output correct pickuplink
	    if($dbpic !== false && $dbpic == $cookiepic) {
		    ?><a href="/offensive/pages/pic.php?id=<?= $dbpic ?>" id="pickUp">Pick up where you left off...</a><?
	    } else if ($dbpic && $cookiepic){
		    ?>Pick up where you left off 
			    (<a href="/offensive/pages/pic.php?id=<?= $cookiepic ?>" id="pickUp">on this computer</a> | 
			    <a href="/offensive/pages/pic.php?id=<?= $dbpic ?>" id="pickUp">on this account</a>)<?
	    } else if ($cookiepic) {
		    ?><a href="/offensive/pages/pic.php?id=<?= $cookiepic ?>" id="pickUp">Pick up where you left off...</a><?
	    } else if ($dbpic) {
	    	?><a href="/offensive/pages/pic.php?id=<?= $dbpic ?>" id="pickUp">Pick up where you left off...</a><?
	    }
    }

    if(!array_key_exists("p", $args) || !is_numeric($args["p"]) || $args["p"] < 1) {
        $args["p"] = 0;
    } else {
        $args["p"]--;
    }
	if(!array_key_exists("limit", $args)) {
		$args["limit"] = $numPerPage;
	}
	if(!array_key_exists("type", $args)) {
		$args["type"] = "image";
	}

    $result = core_getuploads($args);

    // need to organize the result into something that corresponds better with
    // the output loop.

    $output = array();
    for($i = 0; $i < count($result); $i++) {
        if($i % 6 == 0) {
            $output[] = array($result[$i]);
        } else {
            $output[count($output) - 1][] = $result[$i];
        }
    }

?>
<html>
    <head>
        <meta http-equiv=" Content-Type" content="text/html;charset=UTF-8">
        <title>
            thismight.be/offensive - Google Search
        </title>
        <style type="text/css">
            .gac_m{cursor:default;line-height:117%;border:1px solid #666;z-index:99;background:white;position:absolute;margin:0;font-size:17px}
            .gac_m td{line-height:22px}
            .gac_n{padding-top:1px;padding-bottom:1px}
            .gac_b td.gac_c{background:#d5e2ff}
            .gac_b{background:#d5e2ff}
            .gac_a td.gac_f{background:#fff8dd}
            .gac_p{padding:1px 4px 2px 3px}
            .gac_u{padding:0 0 1px 0;line-height:117%;text-align:left}
            .gac_t{width:100%;text-align:left;font-size:17px}
            .gac_bt{width:375px;text-align:center;padding:8px 0 7px}
            .gac_sb{font-size:15px;height:28px;margin:0.2em;-webkit-appearance:button}
            .gac_z{white-space:nowrap;color:#c00}
            .gac_s{height:3px;font-size:1px}
            .gac_c{white-space:nowrap;overflow:hidden;text-align:left;padding-left:3px;padding-right:3px}
            .gac_e{text-align:right;padding:0 3px}
            .gac_d{font-size:11px}
            .fl,a:link.fl,a:visited.fl{color:#77c}
            .gac_h{color:green}
            .gac_j{display:block}
            .gac_l{line-height:18px}
            .gac_x{display:block;line-height:16px}
            .gac_y{font-size:13px}
            .gac_i{color:#666}
            .gac_w img{width:40px;height:40px}
            .gac_w{width:1px}
            .gac_r{color:red}
            .gac_v{padding-bottom:5px}

            body{background:#fff;color:#000;margin:3px 8px}
            #gbar,#guser{font-size:13px;padding-top:1px !important}
            #gbar{float:left;height:22px}
            #guser{padding-bottom:7px !important;text-align:right}
            .gbh,.gbd{border-top:1px solid #c9d7f1;font-size:1px}
            .gbh{height:0;position:absolute;top:24px;width:100%}
            #gbs,.gbm{background:#fff;left:0;position:absolute;text-align:left;visibility:hidden;z-index:1000}
            .gbm{border:1px solid;border-color:#c9d7f1 #36c #36c #a2bae7;z-index:1001}
            .gb1{margin-right:.5em}
            .gb1,.gb3{zoom:1}
            .gb2{display:block;padding:.2em .5em;}
            .gb2,.gb3{text-decoration:none;border-bottom:none}
            a.gb1,a.gb2,a.gb3,a.gb4{color:#00c !important}
            a.gb2:hover{background:#36c;color:#fff !important}
            a.gb1,a.gb2,a.gb3,.link{color:#20c!important}
            .ts{border-collapse:collapse}
            .ts td{padding:0}
            .ti,.bl,form,#res h3{display:inline}
            .ti{display:inline-table}
            .fl:link,.gl,.gl a:link{color:#77c}
            a:link,.w,#prs a:visited,#prs a:active,.q:active,.q:visited{color:#20c}
            .mblink:visited,a:visited{color:#551a8b}
            a:active{color:red}
            .cur{color:#a90a08;font-weight:bold}
            .b{font-weight:bold}
            .j{width:42em;font-size:82%}
            .s{max-width:42em}
            .sl{font-size:82%}
            #gb{text-align:right;padding:1px 0 7px;margin:0}
            .hd{position:absolute;width:1px;height:1px;top:-1000em;overflow:hidden}
            .f,.m,.c h2,#mbEnd h2{color:#676767}
            .a,cite,.cite,.cite:link{color:green;font-style:normal}
            #mbEnd{float:right}
            h1,ol{margin:0;padding:0}
            li.g,body,html,.std,.c h2,#mbEnd h2,h1{font-size:small;font-family:arial,sans-serif}
            .c h2,#mbEnd h2,h1{font-weight:normal}
            #ssb,.clr{clear:both;margin:0 8px}
            #nav a,#nav a:visited,.blk a{color:#000}
            #nav a{display:block}
            #nav .b a,#nav .b a:visited{color:#20c}
            #nav .i{color:#a90a08;font-weight:bold}
            .csb,.ss,#logo span,#rptglbl{background:url(nav_logo8.png) no-repeat;overflow:hidden}
            .csb,.ss{background-position:0 0;height:26px;display:block}
            .ss{background-position:0 -88px;position:absolute;left:0;top:0}
            .cps{height:18px;overflow:hidden;width:114px}
            .mbi{width:13px;height:13px;background-position:-91px -74px;position:relative;top:2px;margin-right:3px}
            #nav td{padding:0;text-align:center}
            #logo{display:block;overflow:hidden;position:relative;width:103px;height:37px;margin:11px 0 7px}
            #logo img{border:none;position:absolute;left:-0px;top:-26px}
            #logo span,.ch{cursor:pointer}
            .lst{font-family:arial,sans-serif;font-size:17px;vertical-align:middle}
            .lsb{-webkit-appearance:button;padding:0 8px;border:1px solid #999;-webkit-border-radius:2px;background:-webkit-gradient(linear,left top,left bottom,from(#fff),to(#ddd));font-family:arial,sans-serif;font-size:15px;height:1.85em;vertical-align:middle}
            .lsb:active{background:-webkit-gradient(linear,left top,left bottom,from(#ccc),to(#ddd))}
            h3,.med{font-size:medium;font-weight:normal;padding:0;margin:0}
            .e{margin:.75em 0}
            .bc a{color:green;text-decoration:none}
            .bc a:hover{text-decoration:underline}
            .slk td{padding-left:40px;padding-top:5px;vertical-align:top}
            .slk div{padding-left:10px;text-indent:-10px}
            .fc{margin-top:.5em;padding-left:3em}
            #mbEnd cite{display:block;text-align:left}
            #mbEnd p{margin:-.5em 0 0 .5em;text-align:center}
            #bsf,#ssb,.blk{border-top:1px solid #6b90da;background:#f0f7f9}
            #bsf{border-bottom:1px solid #6b90da}
            #flp{margin:7px 0}
            #ssb div{float:left;padding:4px 0 0;padding-left:7px;padding-right:.5em}
            #prs a,#prs b{margin-right:.6em}
            #ssb p{text-align:right;white-space:nowrap;margin:.1em 0;padding:.2em}
            #ssb{margin:0 8px 11px;padding:.1em}
            #cnt{max-width:80em;clear:both}
            #mbEnd{background:#fff;padding:0;border-left:11px solid #fff;border-spacing:0;white-space:nowrap}
            #res{padding-right:1em;margin:0 16px}
            .c{background:#fff8dd;margin:0 8px}
            .c li{padding:0 3px 0 8px;margin:0}
            .c .tam,.c .tal{padding-top:12px}
            #mbEnd li{margin:1em 0;padding:0}
            .xsm{font-size:x-small}
            .sm{margin:0 0 0 40px;padding:0}
            ol li{list-style:none}
            .sm li{margin:0}
            .gl,#bsf a,.nobr{white-space:nowrap}
            #mbEnd .med{white-space:normal}
            .sl,.r{display:inline;font-weight:normal;margin:0}
            .r{font-size:medium}
            h4.r{font-size:small}
            .mr{margin-top:-.5em}
            h3.tbpr{margin-top:.3em;margin-bottom:1em}
            img.tbpr {border:0px;width:15px;height:15px;margin-right:3px}
            .jsb{display:block}
            .nojsb{display:none}
            .rt1{background:transparent url(/images/bubble1b.png) no-repeat}
            .rt2{background:transparent url(/images/bubble2.png) repeat 0 0 scroll}
            .sb{background:url(/images/scrollbar.png) repeat scroll 0 0;cursor:pointer;width:14px}
            .rtdm:hover{text-decoration:underline}
            #sc-block .sc{background:#fff;float:left;font-size:2px;margin:0}
            #sc-block .sc.selected{background:#000}
            #sc-block .sc a{border:1px solid #00c;cursor:pointer;display:block;margin:3px;height:16px;width:16px}
            #sc-block .sc.selected a{border-color:#fff;cursor:default}
            #sc-icon{background:#fff;float:left;font-size:1px;margin:0 5px}
            #sc-icon div{background:#c33;float:left;height:6px;margin:1px 0 0 1px;width:6px}
            #sc-dropdown{background:#fff;border:1px solid;border-color:#c9d7f1 #36c #36c #a2bae7;margin-top:7px;padding:3px;position:absolute;visibility:hidden;width:96px;z-index:1}
            .sc-show{border:0;height:15px;width:15px}
            .sc-hide{border:1px solid #00c;height:13px;width:13px}
            .sc-show div{display:block}
            .sc-hide div{display:none}
            #ImgCont.rpop{margin-left:170px;padding-left:0}
            #ImgCont{margin-left:0;zoom:1}
            #rptgl{font-size:82%;cursor:pointer}
            #rptgl span{color:#20c;text-decoration:underline}
            #rptgl span#rphd{display:none}
            #rptgl.rpop span{display:none}
            #rptgl.rpop span#rphd{display:inline}
            #rptglbl{width:13px;height:13px;overflow:hidden;margin-top:1px;margin-right:4px;background:transparent url(nav_logo8.png) no-repeat -91px -74px;background-position:-91px -74px;display:block;float:left}
            #rptglbl.rpop{background-position:-105px -74px}
            #rptgl.rpop span#rptglbl{display:block}
            #rpsp.rpop{width:150px;border-top:0;position:absolute;left:0;background:#fff;padding:0 10px!important;padding-right:3px;display:block;border-right:1px solid #c9d7f1;margin-top:10px}
            #rpsp{display:none}
            #rpsp h2{font-size:12px;margin:8px 0 2px}
            #rpsp ul{padding-left:0;list-style:none;font-size:12px;margin:0;margin-left:8px}
            #rp4a{width:97px;margin-right:2px}
            #ImgContent{height:auto !important;height:420px}
            #ImgContent table{width:99%}
            #ncm{display:none;padding:10px 0 0}
            #ncm .j{width:auto}
            .sc{margin-top:0;padding-top:10px}
            #rpsp .tl-sect,#rpro{margin-bottom:1.2em;font-size:82%}
            .tl b,#rpro b{font-weight:normal;cursor:pointer}
            .tl-sect div {padding-bottom:2px}
            .tl-sect div.tl b{color:#20c;text-decoration:underline}
            #rpro b{color:#77C;text-decoration:underline}
            .tl-sect div.tl-sel{color:#000000;font-weight:bold;cursor:default}
            .tl-sect i,#rpro i{visibility:hidden;font-style:normal}
            .tl-sel i{visibility:visible}
            .tl-sect div.sc {padding:0}
            #sc-block{margin-left:9px;height:45px}
            #sc-block.sc-dis .sc.selected{background-color:#fff}
            #sc-block.sc-dis .sc.selected a{border-color:#00c;cursor:pointer}
            #rpol,#rpex,#rpfl,#rpnf{display:none}
            #rpol.tl-vis,#rpex.tl-vis,#rpfl.tl-vis,#rpnf.tl-vis{display:block}
            #rpex td{font-size:82%;padding:4px 0 0}
            #rpex td.l{padding-right:.385em;width:0}
            #rpex td.u{padding-left:.385em;width:0}
            #rpfl select,#rpol select{margin:2px 0 4px;width:138px}
            #imgtbbc{margin-right:15px;float:left}
            .crumbs{margin-left:0.5em}
            h3{font-size:medium;font-weight:normal;margin:0;padding:0}
            .shrnkhck{font-size:88%}
            #ss-bar{}
            #ss-box{background:#fff;border:1px solid;border-color:#c9d7f1 #36c #36c #a2bae7;left:0;margin-top:.1em;position:absolute;visibility:hidden;z-index:101}
            #ss-box a{display:block;padding:.2em .31em;text-decoration:none}
            #ss-box a:hover{background:#36c;color:#fff!important}
            a.ss-selected{color:#000!important;font-weight:bold}
            a.ss-unselected{color:#00C!important}
            .ss-selected .mark{display:inline}
            .ss-unselected .mark{visibility:hidden}
            #ss-barframe{background:#fff;left:0;position:absolute;visibility:hidden;z-index:100}
            .crumbs{font-size:82%}
            .crumbs i{padding:0 .5em 0 .5em;font-style:normal;font-weight:bold;top:-.12em}
            #ImgContent{padding-top:1em;}
            #ImgContent td{padding-left:18px}
            #tcell{left:-9999px;position:absolute}
            html{overflow-y:scroll}
            .crumbs{font-size:100%;margin-left:0}
            .crumbs i{padding:0 0.5em 0 0}
            #rptgl{font-size:100%;padding:0}
            #rpsp.rpop{margin-top:0}
            #rpsp .tl-sect,#rpro{font-size:100%}
            #imgtbbc{margin-right:7px}
            #cnt{max-width:none}
            #tads{margin-right:8px}
            .tbo #tads{margin:0 8px 0 170px}
            #ImgCont.rpop{margin-left:0}
            #res{margin:0}
            #sc-block .sc a{height:16px;width:16px}
            .shrnkhck{font-size:100%}
            #prs #imgtbbc a, #prs #imgtbbc b{position:static}
            .ri_cb{left:0;margin:6px;position:absolute;top:0;z-index:1}
            .ri_sp{display:-moz-inline-box;display:inline-block;text-align:center;vertical-align:top;margin-bottom:6px}
            .ri_sp img{vertical-align:bottom}
            .g{margin:1em 0}
            .mbl{margin:1em 0 0}
            em{font-weight:bold;font-style:normal}
            .tbi div, #tbp{background:url(nav_logo8.png) no-repeat;overflow:hidden;width:13px;height:13px;}
            #ssb #tbp{background-position:-91px -74px;padding:0;margin-top:1px;margin-left:0.75em;}
            .tbpo,.tbpc{margin-left:3px;margin-right:1em;text-decoration:underline;white-space:nowrap;}
            .tbpc,.tbo .tbpo {display:inline}
            .tbo .tbpc,.tbpo{display:none}
            #prs *{float:left}
            #prs a, #prs b{position:relative;bottom:.05em;margin-right:.3em}
            .std dfn{padding-left:.2em;padding-right:.5em}
            dfn{font-style:normal;font-weight:bold;padding-left:1px;padding-left:2px;position:relative;top:-.12em}
            #tbd{display:none;margin-left:-9.6em;z-index:1}
            .tbo #tads,.tbo #pp,.tbo #tadsb{margin-left:13em}
            .tbo #res{margin-left:170px;}
            .tbo #tbd{width:9.6em;padding:0;left:11px;background:#fff;border-right:1px solid #c9d7f1;position:absolute;display:block;margin-left:0}
            .tbo #mbEnd{width:26%}
            .jsb{display:none}
            .nojsb{display:block}

            div, table {font-size:13px}
        </style>
    </head>
    <body id="gsr" topmargin="3" marginheight="3" class="   ">
        <textarea id="csi" style="display:none">
        </textarea>
        <noscript>
        </noscript>
        <div id="gbar">
            <nobr>
                <a href="/offensive/?c=discussions" class="gb1">Web</a>
 
                <b class="gb1">Images</b>
 
                <a href="/offensive/?c=audio" class="gb1">Audio</a>
 
                <a href="/offensive/?c=map" class="gb1">Map</a>
 
                <a href="/offensive/?c=changeblog" class="gb1">News</a>
 
                <a href="http://www.timemachinists.com/tmbs.html" class="gb1">Shopping</a>
 
                <a href="/offensive/?c=yearbook" class="gb1">Yearbook</a>
 
                <a href="/offensive/?c=hof" class="gb3"><u>more</u><small>▼</small></a>
            </nobr>
        </div>
        <div id="guser" width="100%">
            <nobr>
                <span id="gbn" class="gbi">
                </span>
                <span id="gbf" class="gbf">
                </span>
                <b class="gb4">
                    <?= $me->username() ?>@thismight.be
                </b>
                | 
                <span id="gbe">
                </span>
                <a href="/offensive/?c=settings" class="gb3"><u>Settings</u> <small>▼</small><div id="gbs" style="left: auto; right: 67px; top: 24px; visibility: hidden; width: 157px; height: 40px; "></div></a>
                | 
                <a href="/offensive/logout.php" class="gb4">Sign out</a>
            </nobr>
        </div>
        <div class="gbh" style="left:0">
        </div>
        <div class="gbh" style="right:0">
        </div>
        <div id="cnt">
            <form id="tsf" name="gs" method="GET" action="/offensive/?c=search">
				<input type="hidden" name="c" value="findfile">
                <table id="sft" class="ts" style="clear:both;margin:19px 16px 20px 15px">
                    <tbody>
                        <tr valign="top">
                            <td>
                                <h1>
                                    <a id="logo" href="http://www.google.com/webhp" title="Go to Google Home">
                                        Google
                                        <img width="164" height="106" src="nav_logo8.png" alt="">
                                    </a>
                                </h1>
                            </td>
                            <td id="sff" style="padding:1px 3px 7px;padding-left:16px;width:100%">
                                <table class="ts" style="margin:12px 0 3px">
                                    <tbody>
                                        <tr>
                                            <td nowrap="">
                                                <input autocomplete="off" class="lst" type="text" name="findfile" size="41" maxlength="2048" value="thismight.be/offensive" title="Search" spellcheck="false">
 
                                                <input type="submit" name="btnG" class="lsb" style="margin:0 2px 0 5px" value="Search">
                                            </td>
                                            <td style="padding:0 6px" class="nobr xsm">
                                                <a href="/offensive/?c=search">
                                                    Advanced Search
                                                </a>
                                                <br>
                                            </td>
                                        </tr>
                                    </tbody>
                                </table>
 
                                <div id="ss-bar" style="position:absolute;white-space:nowrap;z-index:98">
                                    <div style="float:left">
                                        SafeSearch:&nbsp;
                                    </div>
                                    <div id="ss-status" style="float:left;position:relative">
                                        <a class="gb3" style="cursor:pointer;padding-bottom:0" href="/offensive/?c=settings"><u><?
    if($me->getPref("hide_tmbo") && $me->getPref("hide_nsfw")) echo "Strict";
    else if(!$me->getPref("hide_tmbo") && !$me->getPref("hide_nsfw")) echo "Off";
    else echo "Moderate";
                                    ?></u> <small>▼</small></a>
                                        <div id="ss-barframe" style="width: 174px; height: 102px; position: absolute; top: 16px; visibility: hidden; ">
                                        </div>
                                    </div>
                                </div>
                            </td>
                        </tr>
                    </tbody>
                </table>
                <table cellpadding="0" cellspacing="0" class="gac_m" style="visibility: hidden; left: 144px; top: 85px; width: 377px; ">
                </table>
            </form>
            <div style="display:none">
            </div>
            <div id="ssb">
                <div id="prs">
                    <span id="imgtbbc">
                        <span class="crumbs">
                            <a href="/offensive/?c=discussions" class="bcl">Web</a>
                            <i>
                                ›
                            </i>
                            <b>
                                Images
                            </b>
                        </span>
                    </span>
                    <span id="rptgl" class="">
                        <span id="rptglbl" class="">
                        </span>
                        <span>
                            <!-- <a href="/offensive/?c=settings">Show options...</a> -->
                            <?= pickuplink() ?>
                        </span>
                    </span>
                </div>
                <p id="resultStats">
                    &nbsp;Results 
                    <b>
                        <span id="lowerLimit">
                            <?= ($args['p'] * $numPerPage + 1) ?>
                        </span>
                    </b>
                    - 
                    <b>
                        <span id="upperLimit">
                            <?= ($args['p'] * $numPerPage + count($result)) ?>
                        </span>
                    </b>
                    of about 
                    <b>
                        <span id="maxLimit">
                            270,000
                        </span>
                    </b>
                    for <b>thismight.be/offensive</b>.  (
                    <b>
                        <?= number_format(time_end($ptime), 2) ?>
                    </b>
                    seconds)&nbsp;
                </p>
            </div>
            
            <div id="res" class="med">
                <!--a-->
                <h2 class="hd">
                    Search Results
                </h2>
                <div>
                    <ol>
                        <textarea id="rpst" style="display:none">
                        </textarea>
                        <div id="ImgCont" class="">
                            <div id="ImgContent">
                                <table width="100%" class="ts" id="imgtb">
                                    <tbody>
                                        <?
                                            $first = true;
                                            $total = 0;
                                            foreach($output as $row) {
                                                echo "<tr>";
                                                for($col = 0; $col < count($row); $col++) {
                                                    $upload = $row[$col];
                                        ?>
                                            <td id="tDataImage<?= $total++ ?>" style="<?= ($first ? "padding-top:0px" : "padding-top: 20px") ?>" align="left" nowrap="" valign="bottom" width="16.666666%">
                                                <a href="/offensive/pages/pic.php?id=<?= $upload->id() ?>">
                                                    <img style="border:1px solid;vertical-align:bottom" src="<?= ($upload->filtered() ? "/offensive/graphics/th-filtered.gif" : $upload->thumbURL()) ?>">
                                                </a>
                                            </td>
                                        <?      }
                                                for(;$col < 6; $col++) {
                                                    echo "<td></td>";
                                                }
                                                echo "</tr><tr>";
                                                for($col = 0; $col < count($row); $col++) {
                                                    $upload = $row[$col];
                                        ?>
                                            <td id="tDataText0" align="left" valign="top" width="16.666666%">
                                                <div class="std">
                                                    <?
                                                        $filename = substr($upload->filename(), 0, strrpos($upload->filename(), "."));
							$filename = str_replace(array("<", ">"), array("[", "]"), strip_tags(str_replace(array("[", "]"), array("<", ">"), $filename)));
                                                        echo (strlen($filename) > 22 ? substr($filename, 0, 22) : $filename);
                                                    if($upload->file()) {
                                                    ?>
                                                    <div class="f">
                                        <?
                                            $info = getimagesize($upload->file());
                                            echo $info[0]." x ".$info[1]." - ".
                                                 floor(filesize($upload->file()) / 1024).
                                                 "k&nbsp;-&nbsp;".
(substr($upload->filename(), strrpos($upload->filename(), ".") + 1));
                                        ?>
                                            </div>
                                                <? } ?>
                                                    <div class="a">
                                                        <cite style="font-style:normal">
                                                            <?= strtolower($upload->uploader()->username()).randtld($upload->uploader()->username()) ?>
                                                        </cite>
                                                    </div>
                                                </div>
                                            </td>
                                        <?      }
                                                for(;$col < 6; $col++) {
                                                    echo "<td></td>";
                                                }
                                                echo "</tr>";
                                                if($first == true) $first = false;
                                            }
                                        ?>
                                    </tbody>
                                </table>
                            </div>
                            <div id="rw">
                            </div>
                            <span class="std" id="tcell">
                            </span>
                        </div>
                        <div id="ncm" class="">
                        </div>
                    </ol>
                </div>
                <!--z-->
            </div>
            <br clear="all">
            <div id="frc">
            </div>
            <div id="navcnt">
                <table id="nav" align="center" style="border-collapse:collapse;margin:auto;text-align:center;direction:ltr;margin-bottom:1.4em;">
                    <tbody>
                        <tr valign="top">
                            <td class="b">
<?
    $newargs = $args;
    if($args['p'] == 0) {
?>
                                <span class="csb" style="background-position:-26px 0;width:18px">
                                </span>
<? } else { ?>
                                <a href="./?<?= http_build_query($newargs) ?>"><span class="csb" style="background-position:0 0;margin-left:auto;width:44px"></span><div style="margin-right: 8px;">Previous</div></a>
<? } ?>
                            </td>
<?
    for($i = max(1, $args['p'] - 9); $i <= max(10, $args['p'] + 10); $i++) {
        $newargs['p'] = $i;
        if($i == $args['p'] + 1) {
?>
                            <td class="cur">
                                <span class="csb" style="background-position:-44px 0;width:16px">
                                </span>
<?= $i ?>
                            </td>
<?      } else { ?>
                            <td>
                                <a href="./?<?= http_build_query($newargs) ?>"><span class="csb ch" style="background-position:-60px 0;width:16px"></span><?= $i ?></a>
                            </td>
<?      }
    } ?>
                            <td class="b">
<?
    $newargs['p'] = $args['p'] + 2;
?>
                                <a href="./?<?= http_build_query($newargs) ?>"><span class="csb ch" style="background-position:-76px 0;margin-right:34px;width:66px"></span>Next</a>
                            </td>
                        </tr>
                    </tbody>
                </table>
            </div>
            <div style="height:1px;line-height:0">
            </div>
            <div style="text-align:center;margin-top:1.4em" class="clr">
                <div id="bsf" style="padding:1.8em 0;margin-top:0">
                    <form method="GET" action="/offensive/?c=search">
                        <div>
				            <input type="hidden" name="c" value="findfile">
                            <input class="lst" type="text" name="findfile" size="41" maxlength="2048" value="thismight.be/offensive" title="Search">
     
                            <input type="submit" name="btnG" class="lsb" style="margin:0 2px 0 5px" value="Search">
                        </div>
                    </form>
                </div>
            </div>
            <textarea style="display:none" id="hcache">
            </textarea>
            <div id="xjsd">
            </div>
            <div id="xjsi">
            </div>
        </div>
<? include_once("analytics.inc") ?>
    </body>
</html>
