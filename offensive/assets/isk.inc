<?php

/* this adds an ISK (imgSeek) class to TMBO which powers the content based repost checker

   written by Cor Bosman (cor@tmbo)
*/

class Isk {
 	private $client;			// xmlrpc connection
	private $errno;				// error code
	private $errmsg;
	
	// initializes XMLRPC 
	public function __construct() {
		$config = parse_ini_file("admin/.config");
		$this->client = new xmlrpc_client("/RPC", $config['isk_host'], $config['isk_port']);
	}
	
	// We need to know if ISK contains a DB. If not, fail immediately. 
	// ISK will crash if you try to add images without a DB present
	public function db_exists() {
		$msg = new xmlrpcmsg('getDbList');
		$result = &$this->client->send($msg);
		
		if(!$result->faultCode()) {
			$this->errno = 0;
			$this->errmsg = "";
			$value = $result->value();
			return ($value->arraysize() > 0) ? true : false;
		} else {
			$this->errno = $result->faultCode();
			$this->errmsg = $result->faultString();
		}
	}
	
	// Check if an img exists on the ISK server. 
	// We need this to prevent unnecessary transfers and ISK currently crashes if you 
	// re-upload the same ID.
	public function img_exists($id) {
		$msg = new xmlrpcmsg('isImgOnDb',
	            		array(	php_xmlrpc_encode(1),		// ISK DB ID
	                  		php_xmlrpc_encode($id)		// IMG ID
	                	));
	
		$result = &$this->client->send($msg);
		
		if(!$result->faultCode()) {
			$this->errno = 0;
			$this->errmsg = "";
			$value = $result->value();
			return $value->scalarval();			
		} else {
			$this->errno = $result->faultCode();
			$this->errmsg = $result->faultString();
		} 
	}
	
	// ISK needs an image blob of the image we want to check
	// create img blob
	public function img_blob($path) {
		$image = new Imagick($path);
		$image->flattenImages();
		
		$max_dimension = 300;
		if($image->getImageWidth() > $image->getImageHeight() && $image->getImageHeight() > $max_dimension) {
			$image->resizeImage(0,$max_dimension, imagick::FILTER_UNDEFINED, 1, false);
		} else if($image->getImageWidth() > $max_dimension) {
			$image->resizeImage($max_dimension,0, imagick::FILTER_UNDEFINED, 1, false);
		}
		$blob = $image->getImageBlob();
		$image->destroy();
		
		return $blob;
	}
	
	// remove image from ISK
	public function delete($id) {
		$msg = new xmlrpcmsg('removeImg', 
				array(	php_xmlrpc_encode(1), 					// dbID = 1
					php_xmlrpc_encode($id)					// img id
				));
		$result = &$this->client->send($msg);
		if(!$result->faultCode()) {
			$this->errno = 0;
			$this->errmsg = "";
			return true;
		} else {
			$this->errno = $result->faultCode();
			$this->errmsg = $result->faultString();
		}
	}
	
	// add image to ISK
	public function add($image_path, $fileid, $replace=false) {
		
		// check if ID exists. 
		if($this->img_exists($fileid)) {
			if($replace) {
				$this->delete($fileid);
			} else {
				return false;
			}
		}
		
		if($this->error()) {
			return false;
		}
		
		$blob = $this->img_blob($image_path);
		if($this->error()) {
			return false;
		}

		$this->_add($fileid, $blob);
		if($this->error()) {
			return false;
		}
		
		return true;
	}
	
	private function _add($id, $img) {
		$msg = new xmlrpcmsg('addImgBlob',
				array(	php_xmlrpc_encode(1), 					// dbID = 1
					php_xmlrpc_encode($id),					// img id
					php_xmlrpc_encode(new xmlrpcval($img, 'base64'))	// path to image
				));
		$result = &$this->client->send($msg);
		
		if(!$result->faultCode()) {
			$this->errno = 0;
			$this->errmsg = "";
			return true;
		} else {
			$this->errno = $result->faultCode();
			$this->errmsg = $result->faultString();
		}
	}
	
	// find img
	public function find($img) {
		$msg = new xmlrpcmsg('queryImgBlob',
				array(	php_xmlrpc_encode(1), 					// dbID = 1
					php_xmlrpc_encode(new xmlrpcval($img, 'base64')),	// path to image
					php_xmlrpc_encode(20)					// max 20 hits
				));
		
		$result = &$this->client->send($msg);
		
		$iskIds = array();
		if(!$result->faultCode()) {
			$value = $result->value();
			for ($i = 0; $i < $value->arraysize(); $i++) {
				$pair = $value->arraymem($i);
				$imgid = $pair->arraymem(0)->scalarval();
				$matchpct = $pair->arraymem(1)->scalarval();
				if($matchpct > 70) {
					$iskIds[] = $imgid;
				}
			}
			$this->errno = 0;
			$this->errmsg = "";
		} else {
			$this->errno = $result->faultCode();
			$this->errmsg = $result->faultString();
		}
		
		return $iskIds;
	}
	
	// save DB
	public function save() {
		$msg = new xmlrpcmsg('saveAllDbs');
		$result = &$this->client->send($msg);
		
		if(!$result->faultCode()) {
			$this->errno = 0;
			$this->errmsg = "";
			return true;
		} else {
			$this->errno = $result->faultCode();
			$this->errmsg = $result->faultString();	
		}
	}
	
	// check if there was an error
	public function error() {
		return ($this->errno > 0) ? true : false;
	}
	
	public function error_msg() {
		return $this->errmsg;
	}
}
