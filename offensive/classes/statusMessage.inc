<?php
	class statusMessage {
		public $status;
		public $message;

		function __construct( $status, $message = "" ) {
			$this->status = $status;
			$this->message = $message;
		}
		
	}

?>