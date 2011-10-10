<?

// ID3 code by James Heinrich <info@getid3.org>

define('GETID3_GIF_SIG',     "\x47\x49\x46");  // 'GIF'
define('GETID3_PNG_SIG',     "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A");
define('GETID3_JPG_SIG',     "\xFF\xD8\xFF");
define('GETID3_JPG_SOS',     "\xDA"); // Start Of Scan - image data start
define('GETID3_JPG_SOF0',    "\xC0"); // Start Of Frame N
define('GETID3_JPG_SOF1',    "\xC1"); // N indicates which compression process
define('GETID3_JPG_SOF2',    "\xC2"); // Only SOF0-SOF2 are now in common use
define('GETID3_JPG_SOF3',    "\xC3");
// NB: codes C4 and CC are *not* SOF markers
define('GETID3_JPG_SOF5',    "\xC5");
define('GETID3_JPG_SOF6',    "\xC6");
define('GETID3_JPG_SOF7',    "\xC7");
define('GETID3_JPG_SOF9',    "\xC9");
define('GETID3_JPG_SOF10',   "\xCA");
define('GETID3_JPG_SOF11',   "\xCB");
// NB: codes C4 and CC are *not* SOF markers
define('GETID3_JPG_SOF13',   "\xCD");
define('GETID3_JPG_SOF14',   "\xCE");
define('GETID3_JPG_SOF15',   "\xCF");
define('GETID3_JPG_EOI',     "\xD9"); // End Of Image (end of datastream)

class getid3_lib
{

	function PrintHexBytes($string, $hex=true, $spaces=true, $htmlsafe=true) {
		$returnstring = '';
		for ($i = 0; $i < strlen($string); $i++) {
			if ($hex) {
				$returnstring .= str_pad(dechex(ord($string{$i})), 2, '0', STR_PAD_LEFT);
			} else {
				$returnstring .= ' '.(ereg("[\x20-\x7E]", $string{$i}) ? $string{$i} : '§');
			}
			if ($spaces) {
				$returnstring .= ' ';
			}
		}
		if ($htmlsafe) {
			$returnstring = htmlentities($returnstring);
		}
		return $returnstring;
	}

	function SafeStripSlashes($text) {
		if (get_magic_quotes_gpc()) {
			return stripslashes($text);
		}
		return $text;
	}


	static function trunc($floatnumber) {
		// truncates a floating-point number at the decimal point
		// returns int (if possible, otherwise float)
		if ($floatnumber >= 1) {
			$truncatednumber = floor($floatnumber);
		} elseif ($floatnumber <= -1) {
			$truncatednumber = ceil($floatnumber);
		} else {
			$truncatednumber = 0;
		}
		if ($truncatednumber <= 1073741824) { // 2^30
			$truncatednumber = (int) $truncatednumber;
		}
		return $truncatednumber;
	}


	static function CastAsInt($floatnum) {
		// convert to float if not already
		$floatnum = (float) $floatnum;

		// convert a float to type int, only if possible
		if (getid3_lib::trunc($floatnum) == $floatnum) {
			// it's not floating point
			if ($floatnum <= 1073741824) { // 2^30
				// it's within int range
				$floatnum = (int) $floatnum;
			}
		}
		return $floatnum;
	}


	function DecimalBinary2Float($binarynumerator) {
		$numerator   = getid3_lib::Bin2Dec($binarynumerator);
		$denominator = getid3_lib::Bin2Dec('1'.str_repeat('0', strlen($binarynumerator)));
		return ($numerator / $denominator);
	}


	function NormalizeBinaryPoint($binarypointnumber, $maxbits=52) {
		// http://www.scri.fsu.edu/~jac/MAD3401/Backgrnd/binary.html
		if (strpos($binarypointnumber, '.') === false) {
			$binarypointnumber = '0.'.$binarypointnumber;
		} elseif ($binarypointnumber{0} == '.') {
			$binarypointnumber = '0'.$binarypointnumber;
		}
		$exponent = 0;
		while (($binarypointnumber{0} != '1') || (substr($binarypointnumber, 1, 1) != '.')) {
			if (substr($binarypointnumber, 1, 1) == '.') {
				$exponent--;
				$binarypointnumber = substr($binarypointnumber, 2, 1).'.'.substr($binarypointnumber, 3);
			} else {
				$pointpos = strpos($binarypointnumber, '.');
				$exponent += ($pointpos - 1);
				$binarypointnumber = str_replace('.', '', $binarypointnumber);
				$binarypointnumber = $binarypointnumber{0}.'.'.substr($binarypointnumber, 1);
			}
		}
		$binarypointnumber = str_pad(substr($binarypointnumber, 0, $maxbits + 2), $maxbits + 2, '0', STR_PAD_RIGHT);
		return array('normalized'=>$binarypointnumber, 'exponent'=>(int) $exponent);
	}


	function Float2BinaryDecimal($floatvalue) {
		// http://www.scri.fsu.edu/~jac/MAD3401/Backgrnd/binary.html
		$maxbits = 128; // to how many bits of precision should the calculations be taken?
		$intpart   = getid3_lib::trunc($floatvalue);
		$floatpart = abs($floatvalue - $intpart);
		$pointbitstring = '';
		while (($floatpart != 0) && (strlen($pointbitstring) < $maxbits)) {
			$floatpart *= 2;
			$pointbitstring .= (string) getid3_lib::trunc($floatpart);
			$floatpart -= getid3_lib::trunc($floatpart);
		}
		$binarypointnumber = decbin($intpart).'.'.$pointbitstring;
		return $binarypointnumber;
	}


	function Float2String($floatvalue, $bits) {
		// http://www.scri.fsu.edu/~jac/MAD3401/Backgrnd/ieee-expl.html
		switch ($bits) {
			case 32:
				$exponentbits = 8;
				$fractionbits = 23;
				break;

			case 64:
				$exponentbits = 11;
				$fractionbits = 52;
				break;

			default:
				return false;
				break;
		}
		if ($floatvalue >= 0) {
			$signbit = '0';
		} else {
			$signbit = '1';
		}
		$normalizedbinary  = getid3_lib::NormalizeBinaryPoint(getid3_lib::Float2BinaryDecimal($floatvalue), $fractionbits);
		$biasedexponent    = pow(2, $exponentbits - 1) - 1 + $normalizedbinary['exponent']; // (127 or 1023) +/- exponent
		$exponentbitstring = str_pad(decbin($biasedexponent), $exponentbits, '0', STR_PAD_LEFT);
		$fractionbitstring = str_pad(substr($normalizedbinary['normalized'], 2), $fractionbits, '0', STR_PAD_RIGHT);

		return getid3_lib::BigEndian2String(getid3_lib::Bin2Dec($signbit.$exponentbitstring.$fractionbitstring), $bits % 8, false);
	}


	function LittleEndian2Float($byteword) {
		return getid3_lib::BigEndian2Float(strrev($byteword));
	}


	function BigEndian2Float($byteword) {
		// ANSI/IEEE Standard 754-1985, Standard for Binary Floating Point Arithmetic
		// http://www.psc.edu/general/software/packages/ieee/ieee.html
		// http://www.scri.fsu.edu/~jac/MAD3401/Backgrnd/ieee.html

		$bitword = getid3_lib::BigEndian2Bin($byteword);
		$signbit = $bitword{0};

		switch (strlen($byteword) * 8) {
			case 32:
				$exponentbits = 8;
				$fractionbits = 23;
				break;

			case 64:
				$exponentbits = 11;
				$fractionbits = 52;
				break;

			case 80:
				// 80-bit Apple SANE format
				// http://www.mactech.com/articles/mactech/Vol.06/06.01/SANENormalized/
				$exponentstring = substr($bitword, 1, 15);
				$isnormalized = intval($bitword{16});
				$fractionstring = substr($bitword, 17, 63);
				$exponent = pow(2, getid3_lib::Bin2Dec($exponentstring) - 16383);
				$fraction = $isnormalized + getid3_lib::DecimalBinary2Float($fractionstring);
				$floatvalue = $exponent * $fraction;
				if ($signbit == '1') {
					$floatvalue *= -1;
				}
				return $floatvalue;
				break;

			default:
				return false;
				break;
		}
		$exponentstring = substr($bitword, 1, $exponentbits);
		$fractionstring = substr($bitword, $exponentbits + 1, $fractionbits);
		$exponent = getid3_lib::Bin2Dec($exponentstring);
		$fraction = getid3_lib::Bin2Dec($fractionstring);

		if (($exponent == (pow(2, $exponentbits) - 1)) && ($fraction != 0)) {
			// Not a Number
			$floatvalue = false;
		} elseif (($exponent == (pow(2, $exponentbits) - 1)) && ($fraction == 0)) {
			if ($signbit == '1') {
				$floatvalue = '-infinity';
			} else {
				$floatvalue = '+infinity';
			}
		} elseif (($exponent == 0) && ($fraction == 0)) {
			if ($signbit == '1') {
				$floatvalue = -0;
			} else {
				$floatvalue = 0;
			}
			$floatvalue = ($signbit ? 0 : -0);
		} elseif (($exponent == 0) && ($fraction != 0)) {
			// These are 'unnormalized' values
			$floatvalue = pow(2, (-1 * (pow(2, $exponentbits - 1) - 2))) * getid3_lib::DecimalBinary2Float($fractionstring);
			if ($signbit == '1') {
				$floatvalue *= -1;
			}
		} elseif ($exponent != 0) {
			$floatvalue = pow(2, ($exponent - (pow(2, $exponentbits - 1) - 1))) * (1 + getid3_lib::DecimalBinary2Float($fractionstring));
			if ($signbit == '1') {
				$floatvalue *= -1;
			}
		}
		return (float) $floatvalue;
	}


	static function BigEndian2Int($byteword, $synchsafe=false, $signed=false) {
		$intvalue = 0;
		$bytewordlen = strlen($byteword);
		for ($i = 0; $i < $bytewordlen; $i++) {
			if ($synchsafe) { // disregard MSB, effectively 7-bit bytes
				$intvalue = $intvalue | (ord($byteword{$i}) & 0x7F) << (($bytewordlen - 1 - $i) * 7);
			} else {
				$intvalue += ord($byteword{$i}) * pow(256, ($bytewordlen - 1 - $i));
			}
		}
		if ($signed && !$synchsafe) {
			// synchsafe ints are not allowed to be signed
			switch ($bytewordlen) {
				case 1:
				case 2:
				case 3:
				case 4:
					$signmaskbit = 0x80 << (8 * ($bytewordlen - 1));
					if ($intvalue & $signmaskbit) {
						$intvalue = 0 - ($intvalue & ($signmaskbit - 1));
					}
					break;

				default:
					die('ERROR: Cannot have signed integers larger than 32-bits in getid3_lib::BigEndian2Int()');
					break;
			}
		}
		return getid3_lib::CastAsInt($intvalue);
	}


	function LittleEndian2Int($byteword, $signed=false) {
		return getid3_lib::BigEndian2Int(strrev($byteword), false, $signed);
	}


	function BigEndian2Bin($byteword) {
		$binvalue = '';
		$bytewordlen = strlen($byteword);
		for ($i = 0; $i < $bytewordlen; $i++) {
			$binvalue .= str_pad(decbin(ord($byteword{$i})), 8, '0', STR_PAD_LEFT);
		}
		return $binvalue;
	}


	function BigEndian2String($number, $minbytes=1, $synchsafe=false, $signed=false) {
		if ($number < 0) {
			return false;
		}
		$maskbyte = (($synchsafe || $signed) ? 0x7F : 0xFF);
		$intstring = '';
		if ($signed) {
			if ($minbytes > 4) {
				die('ERROR: Cannot have signed integers larger than 32-bits in getid3_lib::BigEndian2String()');
			}
			$number = $number & (0x80 << (8 * ($minbytes - 1)));
		}
		while ($number != 0) {
			$quotient = ($number / ($maskbyte + 1));
			$intstring = chr(ceil(($quotient - floor($quotient)) * $maskbyte)).$intstring;
			$number = floor($quotient);
		}
		return str_pad($intstring, $minbytes, "\x00", STR_PAD_LEFT);
	}


	function Dec2Bin($number) {
		while ($number >= 256) {
			$bytes[] = (($number / 256) - (floor($number / 256))) * 256;
			$number = floor($number / 256);
		}
		$bytes[] = $number;
		$binstring = '';
		for ($i = 0; $i < count($bytes); $i++) {
			$binstring = (($i == count($bytes) - 1) ? decbin($bytes[$i]) : str_pad(decbin($bytes[$i]), 8, '0', STR_PAD_LEFT)).$binstring;
		}
		return $binstring;
	}


	function Bin2Dec($binstring, $signed=false) {
		$signmult = 1;
		if ($signed) {
			if ($binstring{0} == '1') {
				$signmult = -1;
			}
			$binstring = substr($binstring, 1);
		}
		$decvalue = 0;
		for ($i = 0; $i < strlen($binstring); $i++) {
			$decvalue += ((int) substr($binstring, strlen($binstring) - $i - 1, 1)) * pow(2, $i);
		}
		return getid3_lib::CastAsInt($decvalue * $signmult);
	}


	function Bin2String($binstring) {
		// return 'hi' for input of '0110100001101001'
		$string = '';
		$binstringreversed = strrev($binstring);
		for ($i = 0; $i < strlen($binstringreversed); $i += 8) {
			$string = chr(getid3_lib::Bin2Dec(strrev(substr($binstringreversed, $i, 8)))).$string;
		}
		return $string;
	}


	function LittleEndian2String($number, $minbytes=1, $synchsafe=false) {
		$intstring = '';
		while ($number > 0) {
			if ($synchsafe) {
				$intstring = $intstring.chr($number & 127);
				$number >>= 7;
			} else {
				$intstring = $intstring.chr($number & 255);
				$number >>= 8;
			}
		}
		return str_pad($intstring, $minbytes, "\x00", STR_PAD_RIGHT);
	}


	function array_merge_clobber($array1, $array2) {
		// written by kcÿhireability*com
		// taken from http://www.php.net/manual/en/function.array-merge-recursive.php
		if (!is_array($array1) || !is_array($array2)) {
			return false;
		}
		$newarray = $array1;
		foreach ($array2 as $key => $val) {
			if (is_array($val) && isset($newarray[$key]) && is_array($newarray[$key])) {
				$newarray[$key] = getid3_lib::array_merge_clobber($newarray[$key], $val);
			} else {
				$newarray[$key] = $val;
			}
		}
		return $newarray;
	}


	function array_merge_noclobber($array1, $array2) {
		if (!is_array($array1) || !is_array($array2)) {
			return false;
		}
		$newarray = $array1;
		foreach ($array2 as $key => $val) {
			if (is_array($val) && isset($newarray[$key]) && is_array($newarray[$key])) {
				$newarray[$key] = getid3_lib::array_merge_noclobber($newarray[$key], $val);
			} elseif (!isset($newarray[$key])) {
				$newarray[$key] = $val;
			}
		}
		return $newarray;
	}


	function fileextension($filename, $numextensions=1) {
		if (strstr($filename, '.')) {
			$reversedfilename = strrev($filename);
			$offset = 0;
			for ($i = 0; $i < $numextensions; $i++) {
				$offset = strpos($reversedfilename, '.', $offset + 1);
				if ($offset === false) {
					return '';
				}
			}
			return strrev(substr($reversedfilename, 0, $offset));
		}
		return '';
	}


	function PlaytimeString($playtimeseconds) {
		$contentseconds = round((($playtimeseconds / 60) - floor($playtimeseconds / 60)) * 60);
		$contentminutes = floor($playtimeseconds / 60);
		if ($contentseconds >= 60) {
			$contentseconds -= 60;
			$contentminutes++;
		}
		return intval($contentminutes).':'.str_pad($contentseconds, 2, 0, STR_PAD_LEFT);
	}


	function image_type_to_mime_type($imagetypeid) {
		// only available in PHP v4.3.0+
		static $image_type_to_mime_type = array();
		if (empty($image_type_to_mime_type)) {
			$image_type_to_mime_type[1]  = 'image/gif';                     // GIF
			$image_type_to_mime_type[2]  = 'image/jpeg';                    // JPEG
			$image_type_to_mime_type[3]  = 'image/png';                     // PNG
			$image_type_to_mime_type[4]  = 'application/x-shockwave-flash'; // Flash
			$image_type_to_mime_type[5]  = 'image/psd';                     // PSD
			$image_type_to_mime_type[6]  = 'image/bmp';                     // BMP
			$image_type_to_mime_type[7]  = 'image/tiff';                    // TIFF: little-endian (Intel)
			$image_type_to_mime_type[8]  = 'image/tiff';                    // TIFF: big-endian (Motorola)
			//$image_type_to_mime_type[9]  = 'image/jpc';                   // JPC
			//$image_type_to_mime_type[10] = 'image/jp2';                   // JPC
			//$image_type_to_mime_type[11] = 'image/jpx';                   // JPC
			//$image_type_to_mime_type[12] = 'image/jb2';                   // JPC
			$image_type_to_mime_type[13] = 'application/x-shockwave-flash'; // Shockwave
			$image_type_to_mime_type[14] = 'image/iff';                     // IFF
		}
		return (isset($image_type_to_mime_type[$imagetypeid]) ? $image_type_to_mime_type[$imagetypeid] : 'application/octet-stream');
	}


	function DateMac2Unix($macdate) {
		// Macintosh timestamp: seconds since 00:00h January 1, 1904
		// UNIX timestamp:      seconds since 00:00h January 1, 1970
		return getid3_lib::CastAsInt($macdate - 2082844800);
	}


	function FixedPoint8_8($rawdata) {
		return getid3_lib::BigEndian2Int(substr($rawdata, 0, 1)) + (float) (getid3_lib::BigEndian2Int(substr($rawdata, 1, 1)) / pow(2, 8));
	}


	function FixedPoint16_16($rawdata) {
		return getid3_lib::BigEndian2Int(substr($rawdata, 0, 2)) + (float) (getid3_lib::BigEndian2Int(substr($rawdata, 2, 2)) / pow(2, 16));
	}


	function FixedPoint2_30($rawdata) {
		$binarystring = getid3_lib::BigEndian2Bin($rawdata);
		return getid3_lib::Bin2Dec(substr($binarystring, 0, 2)) + (float) (getid3_lib::Bin2Dec(substr($binarystring, 2, 30)) / 1073741824);
	}


	function CreateDeepArray($ArrayPath, $Separator, $Value) {
		// assigns $Value to a nested array path:
		//   $foo = getid3_lib::CreateDeepArray('/path/to/my', '/', 'file.txt')
		// is the same as:
		//   $foo = array('path'=>array('to'=>'array('my'=>array('file.txt'))));
		// or
		//   $foo['path']['to']['my'] = 'file.txt';
		while ($ArrayPath && ($ArrayPath{0} == $Separator)) {
			$ArrayPath = substr($ArrayPath, 1);
		}
		if (($pos = strpos($ArrayPath, $Separator)) !== false) {
			$ReturnedArray[substr($ArrayPath, 0, $pos)] = getid3_lib::CreateDeepArray(substr($ArrayPath, $pos + 1), $Separator, $Value);
		} else {
			$ReturnedArray[$ArrayPath] = $Value;
		}
		return $ReturnedArray;
	}

	function array_max($arraydata, $returnkey=false) {
		$maxvalue = false;
		$maxkey = false;
		foreach ($arraydata as $key => $value) {
			if (!is_array($value)) {
				if ($value > $maxvalue) {
					$maxvalue = $value;
					$maxkey = $key;
				}
			}
		}
		return ($returnkey ? $maxkey : $maxvalue);
	}

	function array_min($arraydata, $returnkey=false) {
		$minvalue = false;
		$minkey = false;
		foreach ($arraydata as $key => $value) {
			if (!is_array($value)) {
				if ($value > $minvalue) {
					$minvalue = $value;
					$minkey = $key;
				}
			}
		}
		return ($returnkey ? $minkey : $minvalue);
	}


	function md5_file($file) {

		// md5_file() exists in PHP 4.2.0+.
		if (function_exists('md5_file')) {
			return md5_file($file);
		}

		if (GETID3_OS_ISWINDOWS) {

			$RequiredFiles = array('cygwin1.dll', 'md5sum.exe');
			foreach ($RequiredFiles as $required_file) {
				if (!is_readable(GETID3_HELPERAPPSDIR.$required_file)) {
					die(implode(' and ', $RequiredFiles).' are required in '.GETID3_HELPERAPPSDIR.' for getid3_lib::md5_file() to function under Windows in PHP < v4.2.0');
				}
			}
			$commandline = GETID3_HELPERAPPSDIR.'md5sum.exe "'.str_replace('/', GETID3_OS_DIRSLASH, $file).'"';
			if (ereg("^[\\]?([0-9a-f]{32})", strtolower(`$commandline`), $r)) {
				return $r[1];
			}

		} else {

			// The following works under UNIX only
			$file = str_replace('`', '\\`', $file);
			if (ereg("^([0-9a-f]{32})[ \t\n\r]", `md5sum "$file"`, $r)) {
				return $r[1];
			}

		}
		return false;
	}


	function sha1_file($file) {

		// sha1_file() exists in PHP 4.3.0+.
		if (function_exists('sha1_file')) {
			return sha1_file($file);
		}

		$file = str_replace('`', '\\`', $file);

		if (GETID3_OS_ISWINDOWS) {

			$RequiredFiles = array('cygwin1.dll', 'sha1sum.exe');
			foreach ($RequiredFiles as $required_file) {
				if (!is_readable(GETID3_HELPERAPPSDIR.$required_file)) {
					die(implode(' and ', $RequiredFiles).' are required in '.GETID3_HELPERAPPSDIR.' for getid3_lib::sha1_file() to function under Windows in PHP < v4.3.0');
				}
			}
			$commandline = GETID3_HELPERAPPSDIR.'sha1sum.exe "'.str_replace('/', GETID3_OS_DIRSLASH, $file).'"';
			if (ereg("^sha1=([0-9a-f]{40})", strtolower(`$commandline`), $r)) {
				return $r[1];
			}

		} else {

			$commandline = 'sha1sum "'.$file.'"';
			if (ereg("^([0-9a-f]{40})[ \t\n\r]", strtolower(`$commandline`), $r)) {
				return $r[1];
			}

		}

		return false;
	}


	// Allan Hansen <ahÿartemis*dk>
	// getid3_lib::md5_data() - returns md5sum for a file from startuing position to absolute end position
	function hash_data($file, $offset, $end, $algorithm) {

		switch ($algorithm) {
			case 'md5':
				$hash_function = 'md5_file';
				$unix_call     = 'md5sum';
				$windows_call  = 'md5sum.exe';
				$hash_length   = 32;
				break;

			case 'sha1':
				$hash_function = 'sha1_file';
				$unix_call     = 'sha1sum';
				$windows_call  = 'sha1sum.exe';
				$hash_length   = 40;
				break;

			default:
				die('Invalid algorithm ('.$algorithm.') in getid3_lib::hash_data()');
				break;
		}
		$size = $end - $offset;
		while (true) {
			if (GETID3_OS_ISWINDOWS) {

				// It seems that sha1sum.exe for Windows only works on physical files, does not accept piped data
				// Fall back to create-temp-file method:
				if ($algorithm == 'sha1') {
					break;
				}

				$RequiredFiles = array('cygwin1.dll', 'head.exe', 'tail.exe', $windows_call);
				foreach ($RequiredFiles as $required_file) {
					if (!is_readable(GETID3_HELPERAPPSDIR.$required_file)) {
						// helper apps not available - fall back to old method
						break;
					}
				}
				$commandline  = GETID3_HELPERAPPSDIR.'head.exe -c '.$end.' "'.str_replace('/', GETID3_OS_DIRSLASH, $file).'" | ';
				$commandline .= GETID3_HELPERAPPSDIR.'tail.exe -c '.$size.' | ';
				$commandline .= GETID3_HELPERAPPSDIR.$windows_call;

			} else {

				$commandline  = 'head -c '.$end.' "'.$file.'" | ';
				$commandline .= 'tail -c '.$size.' | ';
				$commandline .= $unix_call;

			}
			if ((bool) ini_get('safe_mode')) {
				$ThisFileInfo['warning'][] = 'PHP running in Safe Mode - backtick operator not available, using slower non-system-call '.$algorithm.' algorithm';
				break;
			}
			return substr(`$commandline`, 0, $hash_length);
		}

		// try to create a temporary file in the system temp directory - invalid dirname should force to system temp dir
		if (($data_filename = tempnam('*', 'getID3')) === false) {
			// can't find anywhere to create a temp file, just die
			return false;
		}

		// Init
		$result = false;

		// copy parts of file
		if ($fp = @fopen($file, 'rb')) {

			if ($fp_data = @fopen($data_filename, 'wb')) {

				fseek($fp, $offset, SEEK_SET);
				$byteslefttowrite = $end - $offset;
				while (($byteslefttowrite > 0) && ($buffer = fread($fp, GETID3_FREAD_BUFFER_SIZE))) {
					$byteswritten = fwrite($fp_data, $buffer, $byteslefttowrite);
					$byteslefttowrite -= $byteswritten;
				}
				fclose($fp_data);
				$result = getid3_lib::$hash_function($data_filename);

			}
			fclose($fp);
		}
		unlink($data_filename);
		return $result;
	}


	static function iconv_fallback_int_utf8($charval) {
		if ($charval < 128) {
			// 0bbbbbbb
			$newcharstring = chr($charval);
		} elseif ($charval < 2048) {
			// 110bbbbb 10bbbbbb
			$newcharstring  = chr(($charval >> 6) | 0xC0);
			$newcharstring .= chr(($charval & 0x3F) | 0x80);
		} elseif ($charval < 65536) {
			// 1110bbbb 10bbbbbb 10bbbbbb
			$newcharstring  = chr(($charval >> 12) | 0xE0);
			$newcharstring .= chr(($charval >>  6) | 0xC0);
			$newcharstring .= chr(($charval & 0x3F) | 0x80);
		} else {
			// 11110bbb 10bbbbbb 10bbbbbb 10bbbbbb
			$newcharstring  = chr(($charval >> 18) | 0xF0);
			$newcharstring .= chr(($charval >> 12) | 0xC0);
			$newcharstring .= chr(($charval >>  6) | 0xC0);
			$newcharstring .= chr(($charval & 0x3F) | 0x80);
		}
		return $newcharstring;
	}

	// ISO-8859-1 => UTF-8
	function iconv_fallback_iso88591_utf8($string, $bom=false) {
		if (function_exists('utf8_encode')) {
			return utf8_encode($string);
		}
		// utf8_encode() unavailable, use getID3()'s iconv_fallback() conversions (possibly PHP is compiled without XML support)
		$newcharstring = '';
		if ($bom) {
			$newcharstring .= "\xEF\xBB\xBF";
		}
		for ($i = 0; $i < strlen($string); $i++) {
			$charval = ord($string{$i});
			$newcharstring .= getid3_lib::iconv_fallback_int_utf8($charval);
		}
		return $newcharstring;
	}

	// ISO-8859-1 => UTF-16BE
	function iconv_fallback_iso88591_utf16be($string, $bom=false) {
		$newcharstring = '';
		if ($bom) {
			$newcharstring .= "\xFE\xFF";
		}
		for ($i = 0; $i < strlen($string); $i++) {
			$newcharstring .= "\x00".$string{$i};
		}
		return $newcharstring;
	}

	// ISO-8859-1 => UTF-16LE
	function iconv_fallback_iso88591_utf16le($string, $bom=false) {
		$newcharstring = '';
		if ($bom) {
			$newcharstring .= "\xFF\xFE";
		}
		for ($i = 0; $i < strlen($string); $i++) {
			$newcharstring .= $string{$i}."\x00";
		}
		return $newcharstring;
	}

	// ISO-8859-1 => UTF-16LE (BOM)
	function iconv_fallback_iso88591_utf16($string) {
		return getid3_lib::iconv_fallback_iso88591_utf16le($string, true);
	}

	// UTF-8 => ISO-8859-1
	function iconv_fallback_utf8_iso88591($string) {
		if (function_exists('utf8_decode')) {
			return utf8_decode($string);
		}
		// utf8_decode() unavailable, use getID3()'s iconv_fallback() conversions (possibly PHP is compiled without XML support)
		$newcharstring = '';
		$offset = 0;
		$stringlength = strlen($string);
		while ($offset < $stringlength) {
			if ((ord($string{$offset}) | 0x07) == 0xF7) {
				// 11110bbb 10bbbbbb 10bbbbbb 10bbbbbb
				$charval = ((ord($string{($offset + 0)}) & 0x07) << 18) &
				           ((ord($string{($offset + 1)}) & 0x3F) << 12) &
				           ((ord($string{($offset + 2)}) & 0x3F) <<  6) &
				            (ord($string{($offset + 3)}) & 0x3F);
				$offset += 4;
			} elseif ((ord($string{$offset}) | 0x0F) == 0xEF) {
				// 1110bbbb 10bbbbbb 10bbbbbb
				$charval = ((ord($string{($offset + 0)}) & 0x0F) << 12) &
				           ((ord($string{($offset + 1)}) & 0x3F) <<  6) &
				            (ord($string{($offset + 2)}) & 0x3F);
				$offset += 3;
			} elseif ((ord($string{$offset}) | 0x1F) == 0xDF) {
				// 110bbbbb 10bbbbbb
				$charval = ((ord($string{($offset + 0)}) & 0x1F) <<  6) &
				            (ord($string{($offset + 1)}) & 0x3F);
				$offset += 2;
			} elseif ((ord($string{$offset}) | 0x7F) == 0x7F) {
				// 0bbbbbbb
				$charval = ord($string{$offset});
				$offset += 1;
			} else {
				// error? throw some kind of warning here?
				$charval = false;
				$offset += 1;
			}
			if ($charval !== false) {
				$newcharstring .= (($charval < 256) ? chr($charval) : '?');
			}
		}
		return $newcharstring;
	}

	// UTF-8 => UTF-16BE
	function iconv_fallback_utf8_utf16be($string, $bom=false) {
		$newcharstring = '';
		if ($bom) {
			$newcharstring .= "\xFE\xFF";
		}
		$offset = 0;
		$stringlength = strlen($string);
		while ($offset < $stringlength) {
			if ((ord($string{$offset}) | 0x07) == 0xF7) {
				// 11110bbb 10bbbbbb 10bbbbbb 10bbbbbb
				$charval = ((ord($string{($offset + 0)}) & 0x07) << 18) &
				           ((ord($string{($offset + 1)}) & 0x3F) << 12) &
				           ((ord($string{($offset + 2)}) & 0x3F) <<  6) &
				            (ord($string{($offset + 3)}) & 0x3F);
				$offset += 4;
			} elseif ((ord($string{$offset}) | 0x0F) == 0xEF) {
				// 1110bbbb 10bbbbbb 10bbbbbb
				$charval = ((ord($string{($offset + 0)}) & 0x0F) << 12) &
				           ((ord($string{($offset + 1)}) & 0x3F) <<  6) &
				            (ord($string{($offset + 2)}) & 0x3F);
				$offset += 3;
			} elseif ((ord($string{$offset}) | 0x1F) == 0xDF) {
				// 110bbbbb 10bbbbbb
				$charval = ((ord($string{($offset + 0)}) & 0x1F) <<  6) &
				            (ord($string{($offset + 1)}) & 0x3F);
				$offset += 2;
			} elseif ((ord($string{$offset}) | 0x7F) == 0x7F) {
				// 0bbbbbbb
				$charval = ord($string{$offset});
				$offset += 1;
			} else {
				// error? throw some kind of warning here?
				$charval = false;
				$offset += 1;
			}
			if ($charval !== false) {
				$newcharstring .= (($charval < 65536) ? getid3_lib::BigEndian2String($charval, 2) : "\x00".'?');
			}
		}
		return $newcharstring;
	}

	// UTF-8 => UTF-16LE
	function iconv_fallback_utf8_utf16le($string, $bom=false) {
		$newcharstring = '';
		if ($bom) {
			$newcharstring .= "\xFF\xFE";
		}
		$offset = 0;
		$stringlength = strlen($string);
		while ($offset < $stringlength) {
			if ((ord($string{$offset}) | 0x07) == 0xF7) {
				// 11110bbb 10bbbbbb 10bbbbbb 10bbbbbb
				$charval = ((ord($string{($offset + 0)}) & 0x07) << 18) &
				           ((ord($string{($offset + 1)}) & 0x3F) << 12) &
				           ((ord($string{($offset + 2)}) & 0x3F) <<  6) &
				            (ord($string{($offset + 3)}) & 0x3F);
				$offset += 4;
			} elseif ((ord($string{$offset}) | 0x0F) == 0xEF) {
				// 1110bbbb 10bbbbbb 10bbbbbb
				$charval = ((ord($string{($offset + 0)}) & 0x0F) << 12) &
				           ((ord($string{($offset + 1)}) & 0x3F) <<  6) &
				            (ord($string{($offset + 2)}) & 0x3F);
				$offset += 3;
			} elseif ((ord($string{$offset}) | 0x1F) == 0xDF) {
				// 110bbbbb 10bbbbbb
				$charval = ((ord($string{($offset + 0)}) & 0x1F) <<  6) &
				            (ord($string{($offset + 1)}) & 0x3F);
				$offset += 2;
			} elseif ((ord($string{$offset}) | 0x7F) == 0x7F) {
				// 0bbbbbbb
				$charval = ord($string{$offset});
				$offset += 1;
			} else {
				// error? maybe throw some warning here?
				$charval = false;
				$offset += 1;
			}
			if ($charval !== false) {
				$newcharstring .= (($charval < 65536) ? getid3_lib::LittleEndian2String($charval, 2) : '?'."\x00");
			}
		}
		return $newcharstring;
	}

	// UTF-8 => UTF-16LE (BOM)
	function iconv_fallback_utf8_utf16($string) {
		return getid3_lib::iconv_fallback_utf8_utf16le($string, true);
	}

	// UTF-16BE => UTF-8
	function iconv_fallback_utf16be_utf8($string) {
		if (substr($string, 0, 2) == "\xFE\xFF") {
			// strip BOM
			$string = substr($string, 2);
		}
		$newcharstring = '';
		for ($i = 0; $i < strlen($string); $i += 2) {
			$charval = getid3_lib::BigEndian2Int(substr($string, $i, 2));
			$newcharstring .= getid3_lib::iconv_fallback_int_utf8($charval);
		}
		return $newcharstring;
	}

	// UTF-16LE => UTF-8
	function iconv_fallback_utf16le_utf8($string) {
		if (substr($string, 0, 2) == "\xFF\xFE") {
			// strip BOM
			$string = substr($string, 2);
		}
		$newcharstring = '';
		for ($i = 0; $i < strlen($string); $i += 2) {
			$charval = getid3_lib::LittleEndian2Int(substr($string, $i, 2));
			$newcharstring .= getid3_lib::iconv_fallback_int_utf8($charval);
		}
		return $newcharstring;
	}

	// UTF-16BE => ISO-8859-1
	function iconv_fallback_utf16be_iso88591($string) {
		if (substr($string, 0, 2) == "\xFE\xFF") {
			// strip BOM
			$string = substr($string, 2);
		}
		$newcharstring = '';
		for ($i = 0; $i < strlen($string); $i += 2) {
			$charval = getid3_lib::BigEndian2Int(substr($string, $i, 2));
			$newcharstring .= (($charval < 256) ? chr($charval) : '?');
		}
		return $newcharstring;
	}

	// UTF-16LE => ISO-8859-1
	function iconv_fallback_utf16le_iso88591($string) {
		if (substr($string, 0, 2) == "\xFF\xFE") {
			// strip BOM
			$string = substr($string, 2);
		}
		$newcharstring = '';
		for ($i = 0; $i < strlen($string); $i += 2) {
			$charval = getid3_lib::LittleEndian2Int(substr($string, $i, 2));
			$newcharstring .= (($charval < 256) ? chr($charval) : '?');
		}
		return $newcharstring;
	}

	// UTF-16 (BOM) => ISO-8859-1
	function iconv_fallback_utf16_iso88591($string) {
		$bom = substr($string, 0, 2);
		if ($bom == "\xFE\xFF") {
			return getid3_lib::iconv_fallback_utf16be_iso88591(substr($string, 2));
		} elseif ($bom == "\xFF\xFE") {
			return getid3_lib::iconv_fallback_utf16le_iso88591(substr($string, 2));
		}
		return $string;
	}

	// UTF-16 (BOM) => UTF-8
	function iconv_fallback_utf16_utf8($string) {
		$bom = substr($string, 0, 2);
		if ($bom == "\xFE\xFF") {
			return getid3_lib::iconv_fallback_utf16be_utf8(substr($string, 2));
		} elseif ($bom == "\xFF\xFE") {
			return getid3_lib::iconv_fallback_utf16le_utf8(substr($string, 2));
		}
		return $string;
	}

	function iconv_fallback($in_charset, $out_charset, $string) {

		if ($in_charset == $out_charset) {
			return $string;
		}

		static $iconv_broken_or_unavailable = array();
		if (is_null(@$iconv_broken_or_unavailable[$in_charset.'_'.$out_charset])) {
			$GETID3_ICONV_TEST_STRING = ' !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ÄÅÇÉÑÖÜáàâäãåçéèêëíìîïñóòôöõúùûü °¢£§•¶ß®©™´¨≠ÆØ∞±≤≥¥µ∂∑∏π∫ªºΩæø¿¡¬√ƒ≈∆«»… ÀÃÕŒœ–—“”‘’÷◊ÿŸ⁄€‹›ﬁﬂ‡·‚„‰ÂÊÁËÈÍÎÏÌÓÔÒÚÛÙıˆ˜¯˘˙˚¸˝˛ˇ';

			// Check iconv()
			if (function_exists('iconv')) {
				if (@iconv($in_charset, 'ISO-8859-1', @iconv('ISO-8859-1', $in_charset, $GETID3_ICONV_TEST_STRING)) == $GETID3_ICONV_TEST_STRING) {
					if (@iconv($out_charset, 'ISO-8859-1', @iconv('ISO-8859-1', $out_charset, $GETID3_ICONV_TEST_STRING)) == $GETID3_ICONV_TEST_STRING) {
						// everything works, use iconv()
						$iconv_broken_or_unavailable[$in_charset.'_'.$out_charset] = false;
					} else {
						// iconv() available, but broken. Use getID3()'s iconv_fallback() conversions instead
						// known issue in PHP v4.1.x
						$iconv_broken_or_unavailable[$in_charset.'_'.$out_charset] = true;
					}
				} else {
					// iconv() available, but broken. Use getID3()'s iconv_fallback() conversions instead
					// known issue in PHP v4.1.x
					$iconv_broken_or_unavailable[$in_charset.'_'.$out_charset] = true;
				}
			} else {
				// iconv() unavailable, use getID3()'s iconv_fallback() conversions
				$iconv_broken_or_unavailable[$in_charset.'_'.$out_charset] = true;
			}
		}

		if ($iconv_broken_or_unavailable[$in_charset.'_'.$out_charset]) {
			static $ConversionFunctionList = array();
			if (empty($ConversionFunctionList)) {
				$ConversionFunctionList['ISO-8859-1']['UTF-8']    = 'iconv_fallback_iso88591_utf8';
				$ConversionFunctionList['ISO-8859-1']['UTF-16']   = 'iconv_fallback_iso88591_utf16';
				$ConversionFunctionList['ISO-8859-1']['UTF-16BE'] = 'iconv_fallback_iso88591_utf16be';
				$ConversionFunctionList['ISO-8859-1']['UTF-16LE'] = 'iconv_fallback_iso88591_utf16le';
				$ConversionFunctionList['UTF-8']['ISO-8859-1']    = 'iconv_fallback_utf8_iso88591';
				$ConversionFunctionList['UTF-8']['UTF-16']        = 'iconv_fallback_utf8_utf16';
				$ConversionFunctionList['UTF-8']['UTF-16BE']      = 'iconv_fallback_utf8_utf16be';
				$ConversionFunctionList['UTF-8']['UTF-16LE']      = 'iconv_fallback_utf8_utf16le';
				$ConversionFunctionList['UTF-16']['ISO-8859-1']   = 'iconv_fallback_utf16_iso88591';
				$ConversionFunctionList['UTF-16']['UTF-8']        = 'iconv_fallback_utf16_utf8';
				$ConversionFunctionList['UTF-16LE']['ISO-8859-1'] = 'iconv_fallback_utf16le_iso88591';
				$ConversionFunctionList['UTF-16LE']['UTF-8']      = 'iconv_fallback_utf16le_utf8';
				$ConversionFunctionList['UTF-16BE']['ISO-8859-1'] = 'iconv_fallback_utf16be_iso88591';
				$ConversionFunctionList['UTF-16BE']['UTF-8']      = 'iconv_fallback_utf16be_utf8';
			}
			if (isset($ConversionFunctionList[strtoupper($in_charset)][strtoupper($out_charset)])) {
				$ConversionFunction = $ConversionFunctionList[strtoupper($in_charset)][strtoupper($out_charset)];
				return getid3_lib::$ConversionFunction($string);
			}
			die('PHP does not have iconv() support - cannot convert from '.$in_charset.' to '.$out_charset);
		}

		if ($converted_string = @iconv($in_charset, $out_charset.'//TRANSLIT', $string)) {
			switch ($out_charset) {
				case 'ISO-8859-1':
					$converted_string = rtrim($converted_string, "\x00");
					break;
			}
			return $converted_string;
		}

		// iconv() may sometimes fail with "illegal character in input string" error message
		// and return an empty string, but returning the unconverted string is more useful
		return $string;
	}


	function MultiByteCharString2HTML($string, $charset='ISO-8859-1') {
		$HTMLstring = '';

		switch ($charset) {
			case 'ISO-8859-1':
			case 'ISO8859-1':
			case 'ISO-8859-15':
			case 'ISO8859-15':
			case 'cp866':
			case 'ibm866':
			case '866':
			case 'cp1251':
			case 'Windows-1251':
			case 'win-1251':
			case '1251':
			case 'cp1252':
			case 'Windows-1252':
			case '1252':
			case 'KOI8-R':
			case 'koi8-ru':
			case 'koi8r':
			case 'BIG5':
			case '950':
			case 'GB2312':
			case '936':
			case 'BIG5-HKSCS':
			case 'Shift_JIS':
			case 'SJIS':
			case '932':
			case 'EUC-JP':
			case 'EUCJP':
				$HTMLstring = htmlentities($string, ENT_COMPAT, $charset);
				break;

			case 'UTF-8':
				$strlen = strlen($string);
				for ($i = 0; $i < $strlen; $i++) {
					$char_ord_val = ord($string{$i});
					$charval = 0;
					if ($char_ord_val < 0x80) {
						$charval = $char_ord_val;
					} elseif ((($char_ord_val & 0xF0) >> 4) == 0x0F) {
						$charval  = (($char_ord_val & 0x07) << 18);
						$charval += ((ord($string{++$i}) & 0x3F) << 12);
						$charval += ((ord($string{++$i}) & 0x3F) << 6);
						$charval +=  (ord($string{++$i}) & 0x3F);
					} elseif ((($char_ord_val & 0xE0) >> 5) == 0x07) {
						$charval  = (($char_ord_val & 0x0F) << 12);
						$charval += ((ord($string{++$i}) & 0x3F) << 6);
						$charval +=  (ord($string{++$i}) & 0x3F);
					} elseif ((($char_ord_val & 0xC0) >> 6) == 0x03) {
						$charval  = (($char_ord_val & 0x1F) << 6);
						$charval += (ord($string{++$i}) & 0x3F);
					}
					if (($charval >= 32) && ($charval <= 127)) {
						$HTMLstring .= chr($charval);
					} else {
						$HTMLstring .= '&#'.$charval.';';
					}
				}
				break;

			case 'UTF-16LE':
				for ($i = 0; $i < strlen($string); $i += 2) {
					$charval = getid3_lib::LittleEndian2Int(substr($string, $i, 2));
					if (($charval >= 32) && ($charval <= 127)) {
						$HTMLstring .= chr($charval);
					} else {
						$HTMLstring .= '&#'.$charval.';';
					}
				}
				break;

			case 'UTF-16BE':
				for ($i = 0; $i < strlen($string); $i += 2) {
					$charval = getid3_lib::BigEndian2Int(substr($string, $i, 2));
					if (($charval >= 32) && ($charval <= 127)) {
						$HTMLstring .= chr($charval);
					} else {
						$HTMLstring .= '&#'.$charval.';';
					}
				}
				break;

			default:
				$HTMLstring = 'ERROR: Character set "'.$charset.'" not supported in MultiByteCharString2HTML()';
				break;
		}
		return $HTMLstring;
	}



	function RGADnameLookup($namecode) {
		static $RGADname = array();
		if (empty($RGADname)) {
			$RGADname[0] = 'not set';
			$RGADname[1] = 'Track Gain Adjustment';
			$RGADname[2] = 'Album Gain Adjustment';
		}

		return (isset($RGADname[$namecode]) ? $RGADname[$namecode] : '');
	}


	function RGADoriginatorLookup($originatorcode) {
		static $RGADoriginator = array();
		if (empty($RGADoriginator)) {
			$RGADoriginator[0] = 'unspecified';
			$RGADoriginator[1] = 'pre-set by artist/producer/mastering engineer';
			$RGADoriginator[2] = 'set by user';
			$RGADoriginator[3] = 'determined automatically';
		}

		return (isset($RGADoriginator[$originatorcode]) ? $RGADoriginator[$originatorcode] : '');
	}


	function RGADadjustmentLookup($rawadjustment, $signbit) {
		$adjustment = $rawadjustment / 10;
		if ($signbit == 1) {
			$adjustment *= -1;
		}
		return (float) $adjustment;
	}


	function RGADgainString($namecode, $originatorcode, $replaygain) {
		if ($replaygain < 0) {
			$signbit = '1';
		} else {
			$signbit = '0';
		}
		$storedreplaygain = intval(round($replaygain * 10));
		$gainstring  = str_pad(decbin($namecode), 3, '0', STR_PAD_LEFT);
		$gainstring .= str_pad(decbin($originatorcode), 3, '0', STR_PAD_LEFT);
		$gainstring .= $signbit;
		$gainstring .= str_pad(decbin($storedreplaygain), 9, '0', STR_PAD_LEFT);

		return $gainstring;
	}

	function RGADamplitude2dB($amplitude) {
		return 20 * log10($amplitude);
	}


	function GetURLImageSize($urlpic) {
		if ($fd = @fopen($urlpic, 'rb')){
			$imgData = fread($fd, filesize($urlpic));
			fclose($fd);
			return getid3_lib::GetDataImageSize($imgData);
		}
		return array('', '', '');
	}


	function GetDataImageSize($imgData) {
		$height = '';
		$width  = '';
		$type   = '';
		if ((substr($imgData, 0, 3) == GETID3_GIF_SIG) && (strlen($imgData) > 10)) {
			$dim = unpack('v2dim', substr($imgData, 6, 4));
			$width  = $dim['dim1'];
			$height = $dim['dim2'];
			$type = 1;
		} elseif ((substr($imgData, 0, 8) == GETID3_PNG_SIG) && (strlen($imgData) > 24)) {
			$dim = unpack('N2dim', substr($imgData, 16, 8));
			$width  = $dim['dim1'];
			$height = $dim['dim2'];
			$type = 3;
		} elseif ((substr($imgData, 0, 3) == GETID3_JPG_SIG) && (strlen($imgData) > 4)) {
			///////////////// JPG CHUNK SCAN ////////////////////
			$imgPos = 2;
			$type = 2;
			$buffer = strlen($imgData) - 2;
			while ($imgPos < strlen($imgData)) {
				// synchronize to the marker 0xFF
				$imgPos = strpos($imgData, 0xFF, $imgPos) + 1;
				$marker = $imgData[$imgPos];
				do {
					$marker = ord($imgData[$imgPos++]);
				} while ($marker == 255);
				// find dimensions of block
				switch (chr($marker)) {
					// Grab width/height from SOF segment (these are acceptable chunk types)
					case GETID3_JPG_SOF0:
					case GETID3_JPG_SOF1:
					case GETID3_JPG_SOF2:
					case GETID3_JPG_SOF3:
					case GETID3_JPG_SOF5:
					case GETID3_JPG_SOF6:
					case GETID3_JPG_SOF7:
					case GETID3_JPG_SOF9:
					case GETID3_JPG_SOF10:
					case GETID3_JPG_SOF11:
					case GETID3_JPG_SOF13:
					case GETID3_JPG_SOF14:
					case GETID3_JPG_SOF15:
						$dim = unpack('n2dim', substr($imgData, $imgPos + 3, 4));
						$height = $dim['dim1'];
						$width  = $dim['dim2'];
						break 2; // found it so exit
					case GETID3_JPG_EOI:
					case GETID3_JPG_SOS:
						return false;       // End loop in case we find one of these markers
					default:            // We're not interested in other markers
						$skiplen = (ord($imgData[$imgPos++]) << 8) + ord($imgData[$imgPos++]) - 2;
						// if the skip is more than what we've read in, read more
						$buffer -= $skiplen;
						if ($buffer < 512) { // if the buffer of data is too low, read more file.
							// $imgData .= fread($fd, $skiplen + 1024);
							// $buffer += $skiplen + 1024;
							return false; // End loop in case we find run out of data
						}
						$imgPos += $skiplen;
						break;
				} // endswitch check marker type
			} // endif loop through JPG chunks
		} // endif chk for valid file types

		return array($width, $height, $type);
	} // end function


	function ImageTypesLookup($imagetypeid) {
		static $ImageTypesLookup = array();
		if (empty($ImageTypesLookup)) {
			$ImageTypesLookup[1]  = 'gif';
			$ImageTypesLookup[2]  = 'jpeg';
			$ImageTypesLookup[3]  = 'png';
			$ImageTypesLookup[4]  = 'swf';
			$ImageTypesLookup[5]  = 'psd';
			$ImageTypesLookup[6]  = 'bmp';
			$ImageTypesLookup[7]  = 'tiff (little-endian)';
			$ImageTypesLookup[8]  = 'tiff (big-endian)';
			$ImageTypesLookup[9]  = 'jpc';
			$ImageTypesLookup[10] = 'jp2';
			$ImageTypesLookup[11] = 'jpx';
			$ImageTypesLookup[12] = 'jb2';
			$ImageTypesLookup[13] = 'swc';
			$ImageTypesLookup[14] = 'iff';
		}
		return (isset($ImageTypesLookup[$imagetypeid]) ? $ImageTypesLookup[$imagetypeid] : '');
	}

	function CopyTagsToComments(&$ThisFileInfo) {
		// Copy all entries from ['tags'] into common ['comments'] and ['comments_html']
		if (!empty($ThisFileInfo['tags'])) {
			foreach ($ThisFileInfo['tags'] as $tagtype => $tagarray) {
				foreach ($tagarray as $tagname => $tagdata) {
					foreach ($tagdata as $key => $value) {
						if (!empty($value)) {
							if (empty($ThisFileInfo['comments'][$tagname])) {

								// fall through and append value

							} elseif ($tagtype == 'id3v1') {

								$newvaluelength = strlen(trim($value));
								foreach ($ThisFileInfo['comments'][$tagname] as $existingkey => $existingvalue) {
									$oldvaluelength = strlen(trim($existingvalue));
									if (($newvaluelength <= $oldvaluelength) && (substr($existingvalue, 0, $newvaluelength) == trim($value))) {
										// new value is identical but shorter-than (or equal-length to) one already in comments - skip
										break 2;
									}
								}

							} else {

								$newvaluelength = strlen(trim($value));
								foreach ($ThisFileInfo['comments'][$tagname] as $existingkey => $existingvalue) {
									$oldvaluelength = strlen(trim($existingvalue));
									if (($newvaluelength > $oldvaluelength) && (substr(trim($value), 0, strlen($existingvalue)) == $existingvalue)) {
										$ThisFileInfo['comments'][$tagname][$existingkey] = trim($value);
										break 2;
									}
								}

							}
							if (empty($ThisFileInfo['comments'][$tagname]) || !in_array(trim($value), $ThisFileInfo['comments'][$tagname])) {
								$ThisFileInfo['comments'][$tagname][] = trim($value);
								if (isset($ThisFileInfo['tags_html'][$tagtype][$tagname][$key])) {
									$ThisFileInfo['comments_html'][$tagname][] = $ThisFileInfo['tags_html'][$tagtype][$tagname][$key];
								}
							}
						}
					}
				}
			}
		}
	}


	static function EmbeddedLookup($key, $begin, $end, $file, $name) {

		// Cached
		static $cache;
		if (isset($cache[$file][$name])) {
			return @$cache[$file][$name][$key];
		}

		// Init
		$keylength  = strlen($key);
		$line_count = $end - $begin - 7;

		// Open php file
		$fp = fopen($file, 'r');

		// Discard $begin lines
		for ($i = 0; $i < ($begin + 3); $i++) {
			fgets($fp, 1024);
		}

		// Loop thru line
		while (0 < $line_count--) {

			// Read line
			$line = ltrim(fgets($fp, 1024), "\t ");

			// METHOD A: only cache the matching key - less memory but slower on next lookup of not-previously-looked-up key
			//$keycheck = substr($line, 0, $keylength);
			//if ($key == $keycheck)  {
			//	$cache[$file][$name][$keycheck] = substr($line, $keylength + 1);
			//	break;
			//}

			// METHOD B: cache all keys in this lookup - more memory but faster on next lookup of not-previously-looked-up key
			//$cache[$file][$name][substr($line, 0, $keylength)] = trim(substr($line, $keylength + 1));
			@list($ThisKey, $ThisValue) = explode("\t", $line, 2);
			$cache[$file][$name][$ThisKey] = trim($ThisValue);
		}

		// Close and return
		fclose($fp);
		return @$cache[$file][$name][$key];
	}

}


class getid3_id3v1
{

	function getid3_id3v1(&$fd, &$ThisFileInfo) {

		fseek($fd, -256, SEEK_END);
		$preid3v1 = fread($fd, 128);
		$id3v1tag = fread($fd, 128);

		if (substr($id3v1tag, 0, 3) == 'TAG') {

			$ThisFileInfo['avdataend'] = $ThisFileInfo['filesize'] - 128;

			$ParsedID3v1['title']   = $this->cutfield(substr($id3v1tag,   3, 30));
			$ParsedID3v1['artist']  = $this->cutfield(substr($id3v1tag,  33, 30));
			$ParsedID3v1['album']   = $this->cutfield(substr($id3v1tag,  63, 30));
			$ParsedID3v1['year']    = $this->cutfield(substr($id3v1tag,  93,  4));
			$ParsedID3v1['comment'] =                 substr($id3v1tag,  97, 30);  // can't remove nulls yet, track detection depends on them
			$ParsedID3v1['genreid'] =             ord(substr($id3v1tag, 127,  1));

			// If second-last byte of comment field is null and last byte of comment field is non-null
			// then this is ID3v1.1 and the comment field is 28 bytes long and the 30th byte is the track number
			if (($id3v1tag{125} === "\x00") && ($id3v1tag{126} !== "\x00")) {
				$ParsedID3v1['track']   = ord(substr($ParsedID3v1['comment'], 29,  1));
				$ParsedID3v1['comment'] =     substr($ParsedID3v1['comment'],  0, 28);
			}
			$ParsedID3v1['comment'] = $this->cutfield($ParsedID3v1['comment']);

			$ParsedID3v1['genre'] = $this->LookupGenreName($ParsedID3v1['genreid']);
			if (!empty($ParsedID3v1['genre'])) {
				unset($ParsedID3v1['genreid']);
			}
			if (empty($ParsedID3v1['genre']) || (@$ParsedID3v1['genre'] == 'Unknown')) {
				unset($ParsedID3v1['genre']);
			}

			foreach ($ParsedID3v1 as $key => $value) {
				$ParsedID3v1['comments'][$key][0] = $value;
			}

			// ID3v1 data is supposed to be padded with NULL characters, but some taggers pad with spaces
			$GoodFormatID3v1tag = $this->GenerateID3v1Tag(
											$ParsedID3v1['title'],
											$ParsedID3v1['artist'],
											$ParsedID3v1['album'],
											$ParsedID3v1['year'],
											$this->LookupGenreID(@$ParsedID3v1['genre']),
											$ParsedID3v1['comment'],
											@$ParsedID3v1['track']);
			$ParsedID3v1['padding_valid'] = true;
			if ($id3v1tag !== $GoodFormatID3v1tag) {
				$ParsedID3v1['padding_valid'] = false;
				$ThisFileInfo['warning'][] = 'Some ID3v1 fields do not use NULL characters for padding';
			}

			$ParsedID3v1['tag_offset_end']   = $ThisFileInfo['filesize'];
			$ParsedID3v1['tag_offset_start'] = $ParsedID3v1['tag_offset_end'] - 128;

			$ThisFileInfo['id3v1'] = $ParsedID3v1;
		}

		if (substr($preid3v1, 0, 3) == 'TAG') {
			// The way iTunes handles tags is, well, brain-damaged.
			// It completely ignores v1 if ID3v2 is present.
			// This goes as far as adding a new v1 tag *even if there already is one*

			// A suspected double-ID3v1 tag has been detected, but it could be that
			// the "TAG" identifier is a legitimate part of an APE or Lyrics3 tag
			if (substr($preid3v1, 96, 8) == 'APETAGEX') {
				// an APE tag footer was found before the last ID3v1, assume false "TAG" synch
			} elseif (substr($preid3v1, 119, 6) == 'LYRICS') {
				// a Lyrics3 tag footer was found before the last ID3v1, assume false "TAG" synch
			} else {
				// APE and Lyrics3 footers not found - assume double ID3v1
				$ThisFileInfo['warning'][] = 'Duplicate ID3v1 tag detected - this has been known to happen with iTunes';
				$ThisFileInfo['avdataend'] -= 128;
			}
		}

		return true;
	}

	function cutfield($str) {
		return trim(substr($str, 0, strcspn($str, "\x00")));
	}

	function ArrayOfGenres($allowSCMPXextended=false) {
		static $GenreLookup = array(
			0    => 'Blues',
			1    => 'Classic Rock',
			2    => 'Country',
			3    => 'Dance',
			4    => 'Disco',
			5    => 'Funk',
			6    => 'Grunge',
			7    => 'Hip-Hop',
			8    => 'Jazz',
			9    => 'Metal',
			10   => 'New Age',
			11   => 'Oldies',
			12   => 'Other',
			13   => 'Pop',
			14   => 'R&B',
			15   => 'Rap',
			16   => 'Reggae',
			17   => 'Rock',
			18   => 'Techno',
			19   => 'Industrial',
			20   => 'Alternative',
			21   => 'Ska',
			22   => 'Death Metal',
			23   => 'Pranks',
			24   => 'Soundtrack',
			25   => 'Euro-Techno',
			26   => 'Ambient',
			27   => 'Trip-Hop',
			28   => 'Vocal',
			29   => 'Jazz+Funk',
			30   => 'Fusion',
			31   => 'Trance',
			32   => 'Classical',
			33   => 'Instrumental',
			34   => 'Acid',
			35   => 'House',
			36   => 'Game',
			37   => 'Sound Clip',
			38   => 'Gospel',
			39   => 'Noise',
			40   => 'Alt. Rock',
			41   => 'Bass',
			42   => 'Soul',
			43   => 'Punk',
			44   => 'Space',
			45   => 'Meditative',
			46   => 'Instrumental Pop',
			47   => 'Instrumental Rock',
			48   => 'Ethnic',
			49   => 'Gothic',
			50   => 'Darkwave',
			51   => 'Techno-Industrial',
			52   => 'Electronic',
			53   => 'Pop-Folk',
			54   => 'Eurodance',
			55   => 'Dream',
			56   => 'Southern Rock',
			57   => 'Comedy',
			58   => 'Cult',
			59   => 'Gangsta Rap',
			60   => 'Top 40',
			61   => 'Christian Rap',
			62   => 'Pop/Funk',
			63   => 'Jungle',
			64   => 'Native American',
			65   => 'Cabaret',
			66   => 'New Wave',
			67   => 'Psychedelic',
			68   => 'Rave',
			69   => 'Showtunes',
			70   => 'Trailer',
			71   => 'Lo-Fi',
			72   => 'Tribal',
			73   => 'Acid Punk',
			74   => 'Acid Jazz',
			75   => 'Polka',
			76   => 'Retro',
			77   => 'Musical',
			78   => 'Rock & Roll',
			79   => 'Hard Rock',
			80   => 'Folk',
			81   => 'Folk/Rock',
			82   => 'National Folk',
			83   => 'Swing',
			84   => 'Fast-Fusion',
			85   => 'Bebob',
			86   => 'Latin',
			87   => 'Revival',
			88   => 'Celtic',
			89   => 'Bluegrass',
			90   => 'Avantgarde',
			91   => 'Gothic Rock',
			92   => 'Progressive Rock',
			93   => 'Psychedelic Rock',
			94   => 'Symphonic Rock',
			95   => 'Slow Rock',
			96   => 'Big Band',
			97   => 'Chorus',
			98   => 'Easy Listening',
			99   => 'Acoustic',
			100  => 'Humour',
			101  => 'Speech',
			102  => 'Chanson',
			103  => 'Opera',
			104  => 'Chamber Music',
			105  => 'Sonata',
			106  => 'Symphony',
			107  => 'Booty Bass',
			108  => 'Primus',
			109  => 'Porn Groove',
			110  => 'Satire',
			111  => 'Slow Jam',
			112  => 'Club',
			113  => 'Tango',
			114  => 'Samba',
			115  => 'Folklore',
			116  => 'Ballad',
			117  => 'Power Ballad',
			118  => 'Rhythmic Soul',
			119  => 'Freestyle',
			120  => 'Duet',
			121  => 'Punk Rock',
			122  => 'Drum Solo',
			123  => 'A Cappella',
			124  => 'Euro-House',
			125  => 'Dance Hall',
			126  => 'Goa',
			127  => 'Drum & Bass',
			128  => 'Club-House',
			129  => 'Hardcore',
			130  => 'Terror',
			131  => 'Indie',
			132  => 'BritPop',
			133  => 'Negerpunk',
			134  => 'Polsk Punk',
			135  => 'Beat',
			136  => 'Christian Gangsta Rap',
			137  => 'Heavy Metal',
			138  => 'Black Metal',
			139  => 'Crossover',
			140  => 'Contemporary Christian',
			141  => 'Christian Rock',
			142  => 'Merengue',
			143  => 'Salsa',
			144  => 'Trash Metal',
			145  => 'Anime',
			146  => 'JPop',
			147  => 'Synthpop',

			255  => 'Unknown',

			'CR' => 'Cover',
			'RX' => 'Remix'
		);

		static $GenreLookupSCMPX = array();
		if ($allowSCMPXextended && empty($GenreLookupSCMPX)) {
			$GenreLookupSCMPX = $GenreLookup;
			// http://www.geocities.co.jp/SiliconValley-Oakland/3664/alittle.html#GenreExtended
			// Extended ID3v1 genres invented by SCMPX
			// Note that 255 "Japanese Anime" conflicts with standard "Unknown"
			$GenreLookupSCMPX[240] = 'Sacred';
			$GenreLookupSCMPX[241] = 'Northern Europe';
			$GenreLookupSCMPX[242] = 'Irish & Scottish';
			$GenreLookupSCMPX[243] = 'Scotland';
			$GenreLookupSCMPX[244] = 'Ethnic Europe';
			$GenreLookupSCMPX[245] = 'Enka';
			$GenreLookupSCMPX[246] = 'Children\'s Song';
			$GenreLookupSCMPX[247] = 'Japanese Sky';
			$GenreLookupSCMPX[248] = 'Japanese Heavy Rock';
			$GenreLookupSCMPX[249] = 'Japanese Doom Rock';
			$GenreLookupSCMPX[250] = 'Japanese J-POP';
			$GenreLookupSCMPX[251] = 'Japanese Seiyu';
			$GenreLookupSCMPX[252] = 'Japanese Ambient Techno';
			$GenreLookupSCMPX[253] = 'Japanese Moemoe';
			$GenreLookupSCMPX[254] = 'Japanese Tokusatsu';
			//$GenreLookupSCMPX[255] = 'Japanese Anime';
		}

		return ($allowSCMPXextended ? $GenreLookupSCMPX : $GenreLookup);
	}

	function LookupGenreName($genreid, $allowSCMPXextended=true) {
		switch ($genreid) {
			case 'RX':
			case 'CR':
				break;
			default:
				$genreid = intval($genreid); // to handle 3 or '3' or '03'
				break;
		}
		$GenreLookup = getid3_id3v1::ArrayOfGenres($allowSCMPXextended);
		return (isset($GenreLookup[$genreid]) ? $GenreLookup[$genreid] : false);
	}

	function LookupGenreID($genre, $allowSCMPXextended=false) {
		$GenreLookup = getid3_id3v1::ArrayOfGenres($allowSCMPXextended);
		$LowerCaseNoSpaceSearchTerm = strtolower(str_replace(' ', '', $genre));
		foreach ($GenreLookup as $key => $value) {
			foreach ($GenreLookup as $key => $value) {
				if (strtolower(str_replace(' ', '', $value)) == $LowerCaseNoSpaceSearchTerm) {
					return $key;
				}
			}
			return false;
		}
		return (isset($GenreLookup[$genreid]) ? $GenreLookup[$genreid] : false);
	}

	function StandardiseID3v1GenreName($OriginalGenre) {
		if (($GenreID = getid3_id3v1::LookupGenreID($OriginalGenre)) !== false) {
			return getid3_id3v1::LookupGenreName($GenreID);
		}
		return $OriginalGenre;
	}

	function GenerateID3v1Tag($title, $artist, $album, $year, $genreid, $comment, $track='') {
		$ID3v1Tag  = 'TAG';
		$ID3v1Tag .= str_pad(trim(substr($title,  0, 30)), 30, "\x00", STR_PAD_RIGHT);
		$ID3v1Tag .= str_pad(trim(substr($artist, 0, 30)), 30, "\x00", STR_PAD_RIGHT);
		$ID3v1Tag .= str_pad(trim(substr($album,  0, 30)), 30, "\x00", STR_PAD_RIGHT);
		$ID3v1Tag .= str_pad(trim(substr($year,   0,  4)),  4, "\x00", STR_PAD_LEFT);
		if (!empty($track) && ($track > 0) && ($track <= 255)) {
			$ID3v1Tag .= str_pad(trim(substr($comment, 0, 28)), 28, "\x00", STR_PAD_RIGHT);
			$ID3v1Tag .= "\x00";
			if (gettype($track) == 'string') {
				$track = (int) $track;
			}
			$ID3v1Tag .= chr($track);
		} else {
			$ID3v1Tag .= str_pad(trim(substr($comment, 0, 30)), 30, "\x00", STR_PAD_RIGHT);
		}
		if (($genreid < 0) || ($genreid > 147)) {
			$genreid = 255; // 'unknown' genre
		}
		switch (gettype($genreid)) {
			case 'string':
			case 'integer':
				$ID3v1Tag .= chr(intval($genreid));
				break;
			default:
				$ID3v1Tag .= chr(255); // 'unknown' genre
				break;
		}

		return $ID3v1Tag;
	}

}


class getid3_id3v2
{

	function getid3_id3v2(&$fd, &$ThisFileInfo, $StartingOffset=0) {
		//    Overall tag structure:
		//        +-----------------------------+
		//        |      Header (10 bytes)      |
		//        +-----------------------------+
		//        |       Extended Header       |
		//        | (variable length, OPTIONAL) |
		//        +-----------------------------+
		//        |   Frames (variable length)  |
		//        +-----------------------------+
		//        |           Padding           |
		//        | (variable length, OPTIONAL) |
		//        +-----------------------------+
		//        | Footer (10 bytes, OPTIONAL) |
		//        +-----------------------------+

		//    Header
		//        ID3v2/file identifier      "ID3"
		//        ID3v2 version              $04 00
		//        ID3v2 flags                (%ab000000 in v2.2, %abc00000 in v2.3, %abcd0000 in v2.4.x)
		//        ID3v2 size             4 * %0xxxxxxx


		// shortcuts
		$ThisFileInfo['id3v2']['header'] = true;
		$thisfile_id3v2                  = &$ThisFileInfo['id3v2'];
		$thisfile_id3v2['flags']         =  array();
		$thisfile_id3v2_flags            = &$thisfile_id3v2['flags'];


		fseek($fd, $StartingOffset, SEEK_SET);
		$header = fread($fd, 10);
		if (substr($header, 0, 3) == 'ID3') {

			$thisfile_id3v2['majorversion'] = ord($header{3});
			$thisfile_id3v2['minorversion'] = ord($header{4});

			// shortcut
			$id3v2_majorversion = &$thisfile_id3v2['majorversion'];

		} else {

			unset($ThisFileInfo['id3v2']);
			return false;

		}

		if ($id3v2_majorversion > 4) { // this script probably won't correctly parse ID3v2.5.x and above (if it ever exists)

			$ThisFileInfo['error'][] = 'this script only parses up to ID3v2.4.x - this tag is ID3v2.'.$id3v2_majorversion.'.'.$thisfile_id3v2['minorversion'];
			return false;

		}

		$id3_flags = ord($header{5});
		switch ($id3v2_majorversion) {
			case 2:
				// %ab000000 in v2.2
				$thisfile_id3v2_flags['unsynch']     = (bool) ($id3_flags & 0x80); // a - Unsynchronisation
				$thisfile_id3v2_flags['compression'] = (bool) ($id3_flags & 0x40); // b - Compression
				break;

			case 3:
				// %abc00000 in v2.3
				$thisfile_id3v2_flags['unsynch']     = (bool) ($id3_flags & 0x80); // a - Unsynchronisation
				$thisfile_id3v2_flags['exthead']     = (bool) ($id3_flags & 0x40); // b - Extended header
				$thisfile_id3v2_flags['experim']     = (bool) ($id3_flags & 0x20); // c - Experimental indicator
				break;

			case 4:
				// %abcd0000 in v2.4
				$thisfile_id3v2_flags['unsynch']     = (bool) ($id3_flags & 0x80); // a - Unsynchronisation
				$thisfile_id3v2_flags['exthead']     = (bool) ($id3_flags & 0x40); // b - Extended header
				$thisfile_id3v2_flags['experim']     = (bool) ($id3_flags & 0x20); // c - Experimental indicator
				$thisfile_id3v2_flags['isfooter']    = (bool) ($id3_flags & 0x10); // d - Footer present
				break;
		}

		$thisfile_id3v2['headerlength'] = getid3_lib::BigEndian2Int(substr($header, 6, 4), 1) + 10; // length of ID3v2 tag in 10-byte header doesn't include 10-byte header length

		$thisfile_id3v2['tag_offset_start'] = $StartingOffset;
		$thisfile_id3v2['tag_offset_end']   = $thisfile_id3v2['tag_offset_start'] + $thisfile_id3v2['headerlength'];

	//    Extended Header
		if (isset($thisfile_id3v2_flags['exthead']) && $thisfile_id3v2_flags['exthead']) {
	//            Extended header size   4 * %0xxxxxxx
	//            Number of flag bytes       $01
	//            Extended Flags             $xx
	//            Where the 'Extended header size' is the size of the whole extended header, stored as a 32 bit synchsafe integer.
			$extheader = fread ($fd, 4);
			$thisfile_id3v2['extheaderlength'] = getid3_lib::BigEndian2Int($extheader, 1);

	//            The extended flags field, with its size described by 'number of flag  bytes', is defined as:
	//                %0bcd0000
	//            b - Tag is an update
	//                Flag data length       $00
	//            c - CRC data present
	//                Flag data length       $05
	//                Total frame CRC    5 * %0xxxxxxx
	//            d - Tag restrictions
	//                Flag data length       $01
			$extheaderflagbytes = fread ($fd, 1);
			$extheaderflags     = fread ($fd, $extheaderflagbytes);
			$id3_exthead_flags = getid3_lib::BigEndian2Bin(substr($header, 5, 1));
			$thisfile_id3v2['exthead_flags']['update']       = substr($id3_exthead_flags, 1, 1);
			$thisfile_id3v2['exthead_flags']['CRC']          = substr($id3_exthead_flags, 2, 1);
			if ($thisfile_id3v2['exthead_flags']['CRC']) {
				$extheaderrawCRC = fread ($fd, 5);
				$thisfile_id3v2['exthead_flags']['CRC'] = getid3_lib::BigEndian2Int($extheaderrawCRC, 1);
			}
			$thisfile_id3v2['exthead_flags']['restrictions'] = substr($id3_exthead_flags, 3, 1);
			if ($thisfile_id3v2['exthead_flags']['restrictions']) {
				// Restrictions           %ppqrrstt
				$extheaderrawrestrictions = fread ($fd, 1);
				$thisfile_id3v2['exthead_flags']['restrictions_tagsize']  = (bindec('11000000') & ord($extheaderrawrestrictions)) >> 6; // p - Tag size restrictions
				$thisfile_id3v2['exthead_flags']['restrictions_textenc']  = (bindec('00100000') & ord($extheaderrawrestrictions)) >> 5; // q - Text encoding restrictions
				$thisfile_id3v2['exthead_flags']['restrictions_textsize'] = (bindec('00011000') & ord($extheaderrawrestrictions)) >> 3; // r - Text fields size restrictions
				$thisfile_id3v2['exthead_flags']['restrictions_imgenc']   = (bindec('00000100') & ord($extheaderrawrestrictions)) >> 2; // s - Image encoding restrictions
				$thisfile_id3v2['exthead_flags']['restrictions_imgsize']  = (bindec('00000011') & ord($extheaderrawrestrictions)) >> 0; // t - Image size restrictions
			}
		} // end extended header



		// create 'encoding' key - used by getid3::HandleAllTags()
		// in ID3v2 every field can have it's own encoding type
		// so force everything to UTF-8 so it can be handled consistantly
		$thisfile_id3v2['encoding'] = 'UTF-8';


	//    Frames

	//        All ID3v2 frames consists of one frame header followed by one or more
	//        fields containing the actual information. The header is always 10
	//        bytes and laid out as follows:
	//
	//        Frame ID      $xx xx xx xx  (four characters)
	//        Size      4 * %0xxxxxxx
	//        Flags         $xx xx

		$sizeofframes = $thisfile_id3v2['headerlength'] - 10; // not including 10-byte initial header
		if (isset($thisfile_id3v2['extheaderlength'])) {
			$sizeofframes -= $thisfile_id3v2['extheaderlength'];
		}
		if (isset($thisfile_id3v2_flags['isfooter']) && $thisfile_id3v2_flags['isfooter']) {
			$sizeofframes -= 10; // footer takes last 10 bytes of ID3v2 header, after frame data, before audio
		}
		if ($sizeofframes > 0) {

			$framedata = fread($fd, $sizeofframes); // read all frames from file into $framedata variable

			//    if entire frame data is unsynched, de-unsynch it now (ID3v2.3.x)
			if (isset($thisfile_id3v2_flags['unsynch']) && $thisfile_id3v2_flags['unsynch'] && ($id3v2_majorversion <= 3)) {
				$framedata = $this->DeUnsynchronise($framedata);
			}
			//        [in ID3v2.4.0] Unsynchronisation [S:6.1] is done on frame level, instead
			//        of on tag level, making it easier to skip frames, increasing the streamability
			//        of the tag. The unsynchronisation flag in the header [S:3.1] indicates that
			//        there exists an unsynchronised frame, while the new unsynchronisation flag in
			//        the frame header [S:4.1.2] indicates unsynchronisation.

			$framedataoffset = 10; // how many bytes into the stream - start from after the 10-byte header
			while (isset($framedata) && (strlen($framedata) > 0)) { // cycle through until no more frame data is left to parse
				if (strlen($framedata) <= $this->ID3v2HeaderLength($id3v2_majorversion)) {
					// insufficient room left in ID3v2 header for actual data - must be padding
					$thisfile_id3v2['padding']['start']  = $framedataoffset;
					$thisfile_id3v2['padding']['length'] = strlen($framedata);
					$thisfile_id3v2['padding']['valid']  = true;
					for ($i = 0; $i < $thisfile_id3v2['padding']['length']; $i++) {
						if ($framedata{$i} != "\x00") {
							$thisfile_id3v2['padding']['valid'] = false;
							$thisfile_id3v2['padding']['errorpos'] = $thisfile_id3v2['padding']['start'] + $i;
							$ThisFileInfo['warning'][] = 'Invalid ID3v2 padding found at offset '.$thisfile_id3v2['padding']['errorpos'].' (the remaining '.($thisfile_id3v2['padding']['length'] - $i).' bytes are considered invalid)';
							break;
						}
					}
					break; // skip rest of ID3v2 header
				}
				if ($id3v2_majorversion == 2) {
					// Frame ID  $xx xx xx (three characters)
					// Size      $xx xx xx (24-bit integer)
					// Flags     $xx xx

					$frame_header = substr($framedata, 0, 6); // take next 6 bytes for header
					$framedata    = substr($framedata, 6);    // and leave the rest in $framedata
					$frame_name   = substr($frame_header, 0, 3);
					$frame_size   = getid3_lib::BigEndian2Int(substr($frame_header, 3, 3), 0);
					$frame_flags  = 0; // not used for anything in ID3v2.2, just set to avoid E_NOTICEs

				} elseif ($id3v2_majorversion > 2) {

					// Frame ID  $xx xx xx xx (four characters)
					// Size      $xx xx xx xx (32-bit integer in v2.3, 28-bit synchsafe in v2.4+)
					// Flags     $xx xx

					$frame_header = substr($framedata, 0, 10); // take next 10 bytes for header
					$framedata    = substr($framedata, 10);    // and leave the rest in $framedata

					$frame_name = substr($frame_header, 0, 4);
					if ($id3v2_majorversion == 3) {
						$frame_size = getid3_lib::BigEndian2Int(substr($frame_header, 4, 4), 0); // 32-bit integer
					} else { // ID3v2.4+
						$frame_size = getid3_lib::BigEndian2Int(substr($frame_header, 4, 4), 1); // 32-bit synchsafe integer (28-bit value)
					}

					if ($frame_size < (strlen($framedata) + 4)) {
						$nextFrameID = substr($framedata, $frame_size, 4);
						if ($this->IsValidID3v2FrameName($nextFrameID, $id3v2_majorversion)) {
							// next frame is OK
						} elseif (($frame_name == "\x00".'MP3') || ($frame_name == "\x00\x00".'MP') || ($frame_name == ' MP3') || ($frame_name == 'MP3e')) {
							// MP3ext known broken frames - "ok" for the purposes of this test
						} elseif (($id3v2_majorversion == 4) && ($this->IsValidID3v2FrameName(substr($framedata, getid3_lib::BigEndian2Int(substr($frame_header, 4, 4), 0), 4), 3))) {
							$ThisFileInfo['warning'][] = 'ID3v2 tag written as ID3v2.4, but with non-synchsafe integers (ID3v2.3 style). Older versions of Helium2 (www.helium2.com) is a known culprit of this. Tag has been parsed as ID3v2.3';
							$id3v2_majorversion = 3;
							$frame_size = getid3_lib::BigEndian2Int(substr($frame_header, 4, 4), 0); // 32-bit integer
						}
					}


					$frame_flags = getid3_lib::BigEndian2Int(substr($frame_header, 8, 2));
				}

				if ((($id3v2_majorversion == 2) && ($frame_name == "\x00\x00\x00")) || ($frame_name == "\x00\x00\x00\x00")) {
					// padding encountered

					$thisfile_id3v2['padding']['start']  = $framedataoffset;
					$thisfile_id3v2['padding']['length'] = strlen($framedata);
					$thisfile_id3v2['padding']['valid']  = true;
					for ($i = 0; $i < $thisfile_id3v2['padding']['length']; $i++) {
						if ($framedata{$i} != "\x00") {
							$thisfile_id3v2['padding']['valid'] = false;
							$thisfile_id3v2['padding']['errorpos'] = $thisfile_id3v2['padding']['start'] + $i;
							$ThisFileInfo['warning'][] = 'Invalid ID3v2 padding found at offset '.$thisfile_id3v2['padding']['errorpos'].' (the remaining '.($thisfile_id3v2['padding']['length'] - $i).' bytes are considered invalid)';
							break;
						}
					}
					break; // skip rest of ID3v2 header
				}

				if ($frame_name == 'COM ') {
					$ThisFileInfo['warning'][] = 'error parsing "'.$frame_name.'" ('.$framedataoffset.' bytes into the ID3v2.'.$id3v2_majorversion.' tag). (ERROR: IsValidID3v2FrameName("'.str_replace("\x00", ' ', $frame_name).'", '.$id3v2_majorversion.'))). [Note: this particular error has been known to happen with tags edited by iTunes (versions "X v2.0.3", "v3.0.1" are known-guilty, probably others too)]';
					$frame_name = 'COMM';
				}
				if (($frame_size <= strlen($framedata)) && ($this->IsValidID3v2FrameName($frame_name, $id3v2_majorversion))) {

					unset($parsedFrame);
					$parsedFrame['frame_name']      = $frame_name;
					$parsedFrame['frame_flags_raw'] = $frame_flags;
					$parsedFrame['data']            = substr($framedata, 0, $frame_size);
					$parsedFrame['datalength']      = getid3_lib::CastAsInt($frame_size);
					$parsedFrame['dataoffset']      = $framedataoffset;

					$this->ParseID3v2Frame($parsedFrame, $ThisFileInfo);
					$thisfile_id3v2[$frame_name][] = $parsedFrame;

					$framedata = substr($framedata, $frame_size);

				} else { // invalid frame length or FrameID

					if ($frame_size <= strlen($framedata)) {

						if ($this->IsValidID3v2FrameName(substr($framedata, $frame_size, 4), $id3v2_majorversion)) {

							// next frame is valid, just skip the current frame
							$framedata = substr($framedata, $frame_size);
							$ThisFileInfo['warning'][] = 'Next ID3v2 frame is valid, skipping current frame.';

						} else {

							// next frame is invalid too, abort processing
							unset($framedata);
							$ThisFileInfo['error'][] = 'Next ID3v2 frame is also invalid, aborting processing.';

						}

					} elseif ($frame_size == strlen($framedata)) {

						// this is the last frame, just skip
						$ThisFileInfo['warning'][] = 'This was the last ID3v2 frame.';

					} else {

						// next frame is invalid too, abort processing
						unset($framedata);
						$ThisFileInfo['warning'][] = 'Invalid ID3v2 frame size, aborting.';

					}
					if (!$this->IsValidID3v2FrameName($frame_name, $id3v2_majorversion)) {

						switch ($frame_name) {
							case "\x00\x00".'MP':
							case "\x00".'MP3':
							case ' MP3':
							case 'MP3e':
							case "\x00".'MP':
							case ' MP':
							case 'MP3':
								$ThisFileInfo['warning'][] = 'error parsing "'.$frame_name.'" ('.$framedataoffset.' bytes into the ID3v2.'.$id3v2_majorversion.' tag). (ERROR: !IsValidID3v2FrameName("'.str_replace("\x00", ' ', $frame_name).'", '.$id3v2_majorversion.'))). [Note: this particular error has been known to happen with tags edited by "MP3ext (www.mutschler.de/mp3ext/)"]';
								break;

							default:
								$ThisFileInfo['warning'][] = 'error parsing "'.$frame_name.'" ('.$framedataoffset.' bytes into the ID3v2.'.$id3v2_majorversion.' tag). (ERROR: !IsValidID3v2FrameName("'.str_replace("\x00", ' ', $frame_name).'", '.$id3v2_majorversion.'))).';
								break;
						}

					} elseif ($frame_size > strlen($framedata)){

						$ThisFileInfo['error'][] = 'error parsing "'.$frame_name.'" ('.$framedataoffset.' bytes into the ID3v2.'.$id3v2_majorversion.' tag). (ERROR: $frame_size ('.$frame_size.') > strlen($framedata) ('.strlen($framedata).')).';

					} else {

						$ThisFileInfo['error'][] = 'error parsing "'.$frame_name.'" ('.$framedataoffset.' bytes into the ID3v2.'.$id3v2_majorversion.' tag).';

					}

				}
				$framedataoffset += ($frame_size + $this->ID3v2HeaderLength($id3v2_majorversion));

			}

		}


	//    Footer

	//    The footer is a copy of the header, but with a different identifier.
	//        ID3v2 identifier           "3DI"
	//        ID3v2 version              $04 00
	//        ID3v2 flags                %abcd0000
	//        ID3v2 size             4 * %0xxxxxxx

		if (isset($thisfile_id3v2_flags['isfooter']) && $thisfile_id3v2_flags['isfooter']) {
			$footer = fread ($fd, 10);
			if (substr($footer, 0, 3) == '3DI') {
				$thisfile_id3v2['footer'] = true;
				$thisfile_id3v2['majorversion_footer'] = ord($footer{3});
				$thisfile_id3v2['minorversion_footer'] = ord($footer{4});
			}
			if ($thisfile_id3v2['majorversion_footer'] <= 4) {
				$id3_flags = ord(substr($footer{5}));
				$thisfile_id3v2_flags['unsynch_footer']  = (bool) ($id3_flags & 0x80);
				$thisfile_id3v2_flags['extfoot_footer']  = (bool) ($id3_flags & 0x40);
				$thisfile_id3v2_flags['experim_footer']  = (bool) ($id3_flags & 0x20);
				$thisfile_id3v2_flags['isfooter_footer'] = (bool) ($id3_flags & 0x10);

				$thisfile_id3v2['footerlength'] = getid3_lib::BigEndian2Int(substr($footer, 6, 4), 1);
			}
		} // end footer

		if (isset($thisfile_id3v2['comments']['genre'])) {
			foreach ($thisfile_id3v2['comments']['genre'] as $key => $value) {
				unset($thisfile_id3v2['comments']['genre'][$key]);
				$thisfile_id3v2['comments'] = getid3_lib::array_merge_noclobber($thisfile_id3v2['comments'], $this->ParseID3v2GenreString($value));
			}
		}

		if (isset($thisfile_id3v2['comments']['track'])) {
			foreach ($thisfile_id3v2['comments']['track'] as $key => $value) {
				if (strstr($value, '/')) {
					list($thisfile_id3v2['comments']['tracknum'][$key], $thisfile_id3v2['comments']['totaltracks'][$key]) = explode('/', $thisfile_id3v2['comments']['track'][$key]);
				}
			}
		}


		// Set avdataoffset
		$ThisFileInfo['avdataoffset'] = $thisfile_id3v2['headerlength'];
		if (isset($thisfile_id3v2['footer'])) {
			$ThisFileInfo['avdataoffset'] += 10;
		}

		return true;
	}


	function ParseID3v2GenreString($genrestring) {
		// Parse genres into arrays of genreName and genreID
		// ID3v2.2.x, ID3v2.3.x: '(21)' or '(4)Eurodisco' or '(51)(39)' or '(55)((I think...)'
		// ID3v2.4.x: '21' $00 'Eurodisco' $00

		$genrestring = trim($genrestring);
		$returnarray = array();
		if (strpos($genrestring, "\x00") !== false) {
			$unprocessed = trim($genrestring); // trailing nulls will cause an infinite loop.
			$genrestring = '';
			while (strpos($unprocessed, "\x00") !== false) {
				// convert null-seperated v2.4-format into v2.3 ()-seperated format
				$endpos = strpos($unprocessed, "\x00");
				$genrestring .= '('.substr($unprocessed, 0, $endpos).')';
				$unprocessed = substr($unprocessed, $endpos + 1);
			}
			unset($unprocessed);
		}
		if (getid3_id3v1::LookupGenreID($genrestring)) {

			$returnarray['genre'][] = $genrestring;

		} else {

			while (strpos($genrestring, '(') !== false) {

				$startpos = strpos($genrestring, '(');
				$endpos   = strpos($genrestring, ')');
				if (substr($genrestring, $startpos + 1, 1) == '(') {
					$genrestring = substr($genrestring, 0, $startpos).substr($genrestring, $startpos + 1);
					$endpos--;
				}
				$element     = substr($genrestring, $startpos + 1, $endpos - ($startpos + 1));
				$genrestring = substr($genrestring, 0, $startpos).substr($genrestring, $endpos + 1);
				if (getid3_id3v1::LookupGenreName($element)) { // $element is a valid genre id/abbreviation

					if (empty($returnarray['genre']) || !in_array(getid3_id3v1::LookupGenreName($element), $returnarray['genre'])) { // avoid duplicate entires
						$returnarray['genre'][] = getid3_id3v1::LookupGenreName($element);
					}

				} else {

					if (empty($returnarray['genre']) || !in_array($element, $returnarray['genre'])) { // avoid duplicate entires
						$returnarray['genre'][] = $element;
					}

				}
			}
		}
		if ($genrestring) {
			if (empty($returnarray['genre']) || !in_array($genrestring, $returnarray['genre'])) { // avoid duplicate entires
				$returnarray['genre'][]   = $genrestring;
			}
		}

		return $returnarray;
	}


	function ParseID3v2Frame(&$parsedFrame, &$ThisFileInfo) {

		// shortcuts
		$id3v2_majorversion = $ThisFileInfo['id3v2']['majorversion'];

		$parsedFrame['framenamelong']  = $this->FrameNameLongLookup($parsedFrame['frame_name']);
		if (empty($parsedFrame['framenamelong'])) {
			unset($parsedFrame['framenamelong']);
		}
		$parsedFrame['framenameshort'] = $this->FrameNameShortLookup($parsedFrame['frame_name']);
		if (empty($parsedFrame['framenameshort'])) {
			unset($parsedFrame['framenameshort']);
		}

		if ($id3v2_majorversion >= 3) { // frame flags are not part of the ID3v2.2 standard
			if ($id3v2_majorversion == 3) {
				//    Frame Header Flags
				//    %abc00000 %ijk00000
				$parsedFrame['flags']['TagAlterPreservation']  = (bool) ($parsedFrame['frame_flags_raw'] & 0x8000); // a - Tag alter preservation
				$parsedFrame['flags']['FileAlterPreservation'] = (bool) ($parsedFrame['frame_flags_raw'] & 0x4000); // b - File alter preservation
				$parsedFrame['flags']['ReadOnly']              = (bool) ($parsedFrame['frame_flags_raw'] & 0x2000); // c - Read only
				$parsedFrame['flags']['compression']           = (bool) ($parsedFrame['frame_flags_raw'] & 0x0080); // i - Compression
				$parsedFrame['flags']['Encryption']            = (bool) ($parsedFrame['frame_flags_raw'] & 0x0040); // j - Encryption
				$parsedFrame['flags']['GroupingIdentity']      = (bool) ($parsedFrame['frame_flags_raw'] & 0x0020); // k - Grouping identity

			} elseif ($id3v2_majorversion == 4) {
				//    Frame Header Flags
				//    %0abc0000 %0h00kmnp
				$parsedFrame['flags']['TagAlterPreservation']  = (bool) ($parsedFrame['frame_flags_raw'] & 0x4000); // a - Tag alter preservation
				$parsedFrame['flags']['FileAlterPreservation'] = (bool) ($parsedFrame['frame_flags_raw'] & 0x2000); // b - File alter preservation
				$parsedFrame['flags']['ReadOnly']              = (bool) ($parsedFrame['frame_flags_raw'] & 0x1000); // c - Read only
				$parsedFrame['flags']['GroupingIdentity']      = (bool) ($parsedFrame['frame_flags_raw'] & 0x0040); // h - Grouping identity
				$parsedFrame['flags']['compression']           = (bool) ($parsedFrame['frame_flags_raw'] & 0x0008); // k - Compression
				$parsedFrame['flags']['Encryption']            = (bool) ($parsedFrame['frame_flags_raw'] & 0x0004); // m - Encryption
				$parsedFrame['flags']['Unsynchronisation']     = (bool) ($parsedFrame['frame_flags_raw'] & 0x0002); // n - Unsynchronisation
				$parsedFrame['flags']['DataLengthIndicator']   = (bool) ($parsedFrame['frame_flags_raw'] & 0x0001); // p - Data length indicator

				// Frame-level de-unsynchronisation - ID3v2.4
				if ($parsedFrame['flags']['Unsynchronisation']) {
					$parsedFrame['data'] = $this->DeUnsynchronise($parsedFrame['data']);
				}
			}

			//    Frame-level de-compression
			if ($parsedFrame['flags']['compression']) {
				$parsedFrame['decompressed_size'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], 0, 4));
				if (!function_exists('gzuncompress')) {
					$ThisFileInfo['warning'][] = 'gzuncompress() support required to decompress ID3v2 frame "'.$parsedFrame['frame_name'].'"';
				} elseif ($decompresseddata = @gzuncompress(substr($parsedFrame['data'], 4))) {
					$parsedFrame['data'] = $decompresseddata;
				} else {
					$ThisFileInfo['warning'][] = 'gzuncompress() failed on compressed contents of ID3v2 frame "'.$parsedFrame['frame_name'].'"';
				}
			}
		}

		if (isset($parsedFrame['datalength']) && ($parsedFrame['datalength'] == 0)) {

			$warning = 'Frame "'.$parsedFrame['frame_name'].'" at offset '.$parsedFrame['dataoffset'].' has no data portion';
			switch ($parsedFrame['frame_name']) {
				case 'WCOM':
					$warning .= ' (this is known to happen with files tagged by RioPort)';
					break;

				default:
					break;
			}
			$ThisFileInfo['warning'][] = $warning;

		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'UFID')) || // 4.1   UFID Unique file identifier
			(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'UFI'))) {  // 4.1   UFI  Unique file identifier
			//   There may be more than one 'UFID' frame in a tag,
			//   but only one with the same 'Owner identifier'.
			// <Header for 'Unique file identifier', ID: 'UFID'>
			// Owner identifier        <text string> $00
			// Identifier              <up to 64 bytes binary data>

			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00");
			$frame_idstring = substr($parsedFrame['data'], 0, $frame_terminatorpos);
			$parsedFrame['ownerid'] = $frame_idstring;
			$parsedFrame['data'] = substr($parsedFrame['data'], $frame_terminatorpos + strlen("\x00"));
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'TXXX')) || // 4.2.2 TXXX User defined text information frame
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'TXX'))) {    // 4.2.2 TXX  User defined text information frame
			//   There may be more than one 'TXXX' frame in each tag,
			//   but only one with the same description.
			// <Header for 'User defined text information frame', ID: 'TXXX'>
			// Text encoding     $xx
			// Description       <text string according to encoding> $00 (00)
			// Value             <text string according to encoding>

			$frame_offset = 0;
			$frame_textencoding = ord(substr($parsedFrame['data'], $frame_offset++, 1));

			if ((($id3v2_majorversion <= 3) && ($frame_textencoding > 1)) || (($id3v2_majorversion == 4) && ($frame_textencoding > 3))) {
				$ThisFileInfo['warning'][] = 'Invalid text encoding byte ('.$frame_textencoding.') in frame "'.$parsedFrame['frame_name'].'" - defaulting to ISO-8859-1 encoding';
			}
			$frame_terminatorpos = strpos($parsedFrame['data'], $this->TextEncodingTerminatorLookup($frame_textencoding), $frame_offset);
			if (ord(substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)), 1)) === 0) {
				$frame_terminatorpos++; // strpos() fooled because 2nd byte of Unicode chars are often 0x00
			}
			$frame_description = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_description) === 0) {
				$frame_description = '';
			}
			$parsedFrame['encodingid']  = $frame_textencoding;
			$parsedFrame['encoding']    = $this->TextEncodingNameLookup($frame_textencoding);

			$parsedFrame['description'] = $frame_description;
			$parsedFrame['data'] = substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)));
			if (!empty($parsedFrame['framenameshort']) && !empty($parsedFrame['data'])) {
				$ThisFileInfo['id3v2']['comments'][$parsedFrame['framenameshort']][] = trim(getid3_lib::iconv_fallback($parsedFrame['encoding'], $ThisFileInfo['id3v2']['encoding'], $parsedFrame['data']));
			}
			unset($parsedFrame['data']);


		} elseif ($parsedFrame['frame_name']{0} == 'T') { // 4.2. T??[?] Text information frame
			//   There may only be one text information frame of its kind in an tag.
			// <Header for 'Text information frame', ID: 'T000' - 'TZZZ',
			// excluding 'TXXX' described in 4.2.6.>
			// Text encoding                $xx
			// Information                  <text string(s) according to encoding>

			$frame_offset = 0;
			$frame_textencoding = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			if ((($id3v2_majorversion <= 3) && ($frame_textencoding > 1)) || (($id3v2_majorversion == 4) && ($frame_textencoding > 3))) {
				$ThisFileInfo['warning'][] = 'Invalid text encoding byte ('.$frame_textencoding.') in frame "'.$parsedFrame['frame_name'].'" - defaulting to ISO-8859-1 encoding';
			}

			$parsedFrame['data'] = (string) substr($parsedFrame['data'], $frame_offset);

			$parsedFrame['encodingid'] = $frame_textencoding;
			$parsedFrame['encoding']   = $this->TextEncodingNameLookup($frame_textencoding);

			if (!empty($parsedFrame['framenameshort']) && !empty($parsedFrame['data'])) {
				$ThisFileInfo['id3v2']['comments'][$parsedFrame['framenameshort']][] = getid3_lib::iconv_fallback($parsedFrame['encoding'], $ThisFileInfo['id3v2']['encoding'], $parsedFrame['data']);
			}

		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'WXXX')) || // 4.3.2 WXXX User defined URL link frame
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'WXX'))) {    // 4.3.2 WXX  User defined URL link frame
			//   There may be more than one 'WXXX' frame in each tag,
			//   but only one with the same description
			// <Header for 'User defined URL link frame', ID: 'WXXX'>
			// Text encoding     $xx
			// Description       <text string according to encoding> $00 (00)
			// URL               <text string>

			$frame_offset = 0;
			$frame_textencoding = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			if ((($id3v2_majorversion <= 3) && ($frame_textencoding > 1)) || (($id3v2_majorversion == 4) && ($frame_textencoding > 3))) {
				$ThisFileInfo['warning'][] = 'Invalid text encoding byte ('.$frame_textencoding.') in frame "'.$parsedFrame['frame_name'].'" - defaulting to ISO-8859-1 encoding';
			}
			$frame_terminatorpos = strpos($parsedFrame['data'], $this->TextEncodingTerminatorLookup($frame_textencoding), $frame_offset);
			if (ord(substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)), 1)) === 0) {
				$frame_terminatorpos++; // strpos() fooled because 2nd byte of Unicode chars are often 0x00
			}
			$frame_description = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);

			if (ord($frame_description) === 0) {
				$frame_description = '';
			}
			$parsedFrame['data'] = substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)));

			$frame_terminatorpos = strpos($parsedFrame['data'], $this->TextEncodingTerminatorLookup($frame_textencoding));
			if (ord(substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)), 1)) === 0) {
				$frame_terminatorpos++; // strpos() fooled because 2nd byte of Unicode chars are often 0x00
			}
			if ($frame_terminatorpos) {
				// there are null bytes after the data - this is not according to spec
				// only use data up to first null byte
				$frame_urldata = (string) substr($parsedFrame['data'], 0, $frame_terminatorpos);
			} else {
				// no null bytes following data, just use all data
				$frame_urldata = (string) $parsedFrame['data'];
			}

			$parsedFrame['encodingid']  = $frame_textencoding;
			$parsedFrame['encoding']    = $this->TextEncodingNameLookup($frame_textencoding);

			$parsedFrame['url']         = $frame_urldata;
			$parsedFrame['description'] = $frame_description;
			if (!empty($parsedFrame['framenameshort']) && $parsedFrame['url']) {
				$ThisFileInfo['id3v2']['comments'][$parsedFrame['framenameshort']][] = getid3_lib::iconv_fallback($parsedFrame['encoding'], $ThisFileInfo['id3v2']['encoding'], $parsedFrame['url']);
			}
			unset($parsedFrame['data']);


		} elseif ($parsedFrame['frame_name']{0} == 'W') { // 4.3. W??? URL link frames
			//   There may only be one URL link frame of its kind in a tag,
			//   except when stated otherwise in the frame description
			// <Header for 'URL link frame', ID: 'W000' - 'WZZZ', excluding 'WXXX'
			// described in 4.3.2.>
			// URL              <text string>

			$parsedFrame['url'] = trim($parsedFrame['data']);
			if (!empty($parsedFrame['framenameshort']) && $parsedFrame['url']) {
				$ThisFileInfo['id3v2']['comments'][$parsedFrame['framenameshort']][] = $parsedFrame['url'];
			}
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion == 3) && ($parsedFrame['frame_name'] == 'IPLS')) || // 4.4  IPLS Involved people list (ID3v2.3 only)
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'IPL'))) {     // 4.4  IPL  Involved people list (ID3v2.2 only)
			//   There may only be one 'IPL' frame in each tag
			// <Header for 'User defined URL link frame', ID: 'IPL'>
			// Text encoding     $xx
			// People list strings    <textstrings>

			$frame_offset = 0;
			$frame_textencoding = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			if ((($id3v2_majorversion <= 3) && ($frame_textencoding > 1)) || (($id3v2_majorversion == 4) && ($frame_textencoding > 3))) {
				$ThisFileInfo['warning'][] = 'Invalid text encoding byte ('.$frame_textencoding.') in frame "'.$parsedFrame['frame_name'].'" - defaulting to ISO-8859-1 encoding';
			}
			$parsedFrame['encodingid'] = $frame_textencoding;
			$parsedFrame['encoding']   = $this->TextEncodingNameLookup($parsedFrame['encodingid']);

			$parsedFrame['data']       = (string) substr($parsedFrame['data'], $frame_offset);
			if (!empty($parsedFrame['framenameshort']) && !empty($parsedFrame['data'])) {
				$ThisFileInfo['id3v2']['comments'][$parsedFrame['framenameshort']][] = getid3_lib::iconv_fallback($parsedFrame['encoding'], $ThisFileInfo['id3v2']['encoding'], $parsedFrame['data']);
			}


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'MCDI')) || // 4.4   MCDI Music CD identifier
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'MCI'))) {     // 4.5   MCI  Music CD identifier
			//   There may only be one 'MCDI' frame in each tag
			// <Header for 'Music CD identifier', ID: 'MCDI'>
			// CD TOC                <binary data>

			if (!empty($parsedFrame['framenameshort']) && !empty($parsedFrame['data'])) {
				$ThisFileInfo['id3v2']['comments'][$parsedFrame['framenameshort']][] = $parsedFrame['data'];
			}


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'ETCO')) || // 4.5   ETCO Event timing codes
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'ETC'))) {     // 4.6   ETC  Event timing codes
			//   There may only be one 'ETCO' frame in each tag
			// <Header for 'Event timing codes', ID: 'ETCO'>
			// Time stamp format    $xx
			//   Where time stamp format is:
			// $01  (32-bit value) MPEG frames from beginning of file
			// $02  (32-bit value) milliseconds from beginning of file
			//   Followed by a list of key events in the following format:
			// Type of event   $xx
			// Time stamp      $xx (xx ...)
			//   The 'Time stamp' is set to zero if directly at the beginning of the sound
			//   or after the previous event. All events MUST be sorted in chronological order.

			$frame_offset = 0;
			$parsedFrame['timestampformat'] = ord(substr($parsedFrame['data'], $frame_offset++, 1));

			while ($frame_offset < strlen($parsedFrame['data'])) {
				$parsedFrame['typeid']    = substr($parsedFrame['data'], $frame_offset++, 1);
				$parsedFrame['type']      = $this->ETCOEventLookup($parsedFrame['typeid']);
				$parsedFrame['timestamp'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, 4));
				$frame_offset += 4;
			}
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'MLLT')) || // 4.6   MLLT MPEG location lookup table
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'MLL'))) {     // 4.7   MLL MPEG location lookup table
			//   There may only be one 'MLLT' frame in each tag
			// <Header for 'Location lookup table', ID: 'MLLT'>
			// MPEG frames between reference  $xx xx
			// Bytes between reference        $xx xx xx
			// Milliseconds between reference $xx xx xx
			// Bits for bytes deviation       $xx
			// Bits for milliseconds dev.     $xx
			//   Then for every reference the following data is included;
			// Deviation in bytes         %xxx....
			// Deviation in milliseconds  %xxx....

			$frame_offset = 0;
			$parsedFrame['framesbetweenreferences'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], 0, 2));
			$parsedFrame['bytesbetweenreferences']  = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], 2, 3));
			$parsedFrame['msbetweenreferences']     = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], 5, 3));
			$parsedFrame['bitsforbytesdeviation']   = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], 8, 1));
			$parsedFrame['bitsformsdeviation']      = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], 9, 1));
			$parsedFrame['data'] = substr($parsedFrame['data'], 10);
			while ($frame_offset < strlen($parsedFrame['data'])) {
				$deviationbitstream .= getid3_lib::BigEndian2Bin(substr($parsedFrame['data'], $frame_offset++, 1));
			}
			$reference_counter = 0;
			while (strlen($deviationbitstream) > 0) {
				$parsedFrame[$reference_counter]['bytedeviation'] = bindec(substr($deviationbitstream, 0, $parsedFrame['bitsforbytesdeviation']));
				$parsedFrame[$reference_counter]['msdeviation']   = bindec(substr($deviationbitstream, $parsedFrame['bitsforbytesdeviation'], $parsedFrame['bitsformsdeviation']));
				$deviationbitstream = substr($deviationbitstream, $parsedFrame['bitsforbytesdeviation'] + $parsedFrame['bitsformsdeviation']);
				$reference_counter++;
			}
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'SYTC')) || // 4.7   SYTC Synchronised tempo codes
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'STC'))) {     // 4.8   STC  Synchronised tempo codes
			//   There may only be one 'SYTC' frame in each tag
			// <Header for 'Synchronised tempo codes', ID: 'SYTC'>
			// Time stamp format   $xx
			// Tempo data          <binary data>
			//   Where time stamp format is:
			// $01  (32-bit value) MPEG frames from beginning of file
			// $02  (32-bit value) milliseconds from beginning of file

			$frame_offset = 0;
			$parsedFrame['timestampformat'] = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$timestamp_counter = 0;
			while ($frame_offset < strlen($parsedFrame['data'])) {
				$parsedFrame[$timestamp_counter]['tempo'] = ord(substr($parsedFrame['data'], $frame_offset++, 1));
				if ($parsedFrame[$timestamp_counter]['tempo'] == 255) {
					$parsedFrame[$timestamp_counter]['tempo'] += ord(substr($parsedFrame['data'], $frame_offset++, 1));
				}
				$parsedFrame[$timestamp_counter]['timestamp'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, 4));
				$frame_offset += 4;
				$timestamp_counter++;
			}
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'USLT')) || // 4.8   USLT Unsynchronised lyric/text transcription
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'ULT'))) {     // 4.9   ULT  Unsynchronised lyric/text transcription
			//   There may be more than one 'Unsynchronised lyrics/text transcription' frame
			//   in each tag, but only one with the same language and content descriptor.
			// <Header for 'Unsynchronised lyrics/text transcription', ID: 'USLT'>
			// Text encoding        $xx
			// Language             $xx xx xx
			// Content descriptor   <text string according to encoding> $00 (00)
			// Lyrics/text          <full text string according to encoding>

			$frame_offset = 0;
			$frame_textencoding = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			if ((($id3v2_majorversion <= 3) && ($frame_textencoding > 1)) || (($id3v2_majorversion == 4) && ($frame_textencoding > 3))) {
				$ThisFileInfo['warning'][] = 'Invalid text encoding byte ('.$frame_textencoding.') in frame "'.$parsedFrame['frame_name'].'" - defaulting to ISO-8859-1 encoding';
			}
			$frame_language = substr($parsedFrame['data'], $frame_offset, 3);
			$frame_offset += 3;
			$frame_terminatorpos = strpos($parsedFrame['data'], $this->TextEncodingTerminatorLookup($frame_textencoding), $frame_offset);
			if (ord(substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)), 1)) === 0) {
				$frame_terminatorpos++; // strpos() fooled because 2nd byte of Unicode chars are often 0x00
			}
			$frame_description = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_description) === 0) {
				$frame_description = '';
			}
			$parsedFrame['data'] = substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)));

			$parsedFrame['encodingid']   = $frame_textencoding;
			$parsedFrame['encoding']     = $this->TextEncodingNameLookup($frame_textencoding);

			$parsedFrame['data']         = $parsedFrame['data'];
			$parsedFrame['language']     = $frame_language;
			$parsedFrame['languagename'] = $this->LanguageLookup($frame_language, false);
			$parsedFrame['description']  = $frame_description;
			if (!empty($parsedFrame['framenameshort']) && !empty($parsedFrame['data'])) {
				$ThisFileInfo['id3v2']['comments'][$parsedFrame['framenameshort']][] = getid3_lib::iconv_fallback($parsedFrame['encoding'], $ThisFileInfo['id3v2']['encoding'], $parsedFrame['data']);
			}
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'SYLT')) || // 4.9   SYLT Synchronised lyric/text
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'SLT'))) {     // 4.10  SLT  Synchronised lyric/text
			//   There may be more than one 'SYLT' frame in each tag,
			//   but only one with the same language and content descriptor.
			// <Header for 'Synchronised lyrics/text', ID: 'SYLT'>
			// Text encoding        $xx
			// Language             $xx xx xx
			// Time stamp format    $xx
			//   $01  (32-bit value) MPEG frames from beginning of file
			//   $02  (32-bit value) milliseconds from beginning of file
			// Content type         $xx
			// Content descriptor   <text string according to encoding> $00 (00)
			//   Terminated text to be synced (typically a syllable)
			//   Sync identifier (terminator to above string)   $00 (00)
			//   Time stamp                                     $xx (xx ...)

			$frame_offset = 0;
			$frame_textencoding = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			if ((($id3v2_majorversion <= 3) && ($frame_textencoding > 1)) || (($id3v2_majorversion == 4) && ($frame_textencoding > 3))) {
				$ThisFileInfo['warning'][] = 'Invalid text encoding byte ('.$frame_textencoding.') in frame "'.$parsedFrame['frame_name'].'" - defaulting to ISO-8859-1 encoding';
			}
			$frame_language = substr($parsedFrame['data'], $frame_offset, 3);
			$frame_offset += 3;
			$parsedFrame['timestampformat'] = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['contenttypeid']   = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['contenttype']     = $this->SYTLContentTypeLookup($parsedFrame['contenttypeid']);
			$parsedFrame['encodingid']      = $frame_textencoding;
			$parsedFrame['encoding']        = $this->TextEncodingNameLookup($frame_textencoding);

			$parsedFrame['language']        = $frame_language;
			$parsedFrame['languagename']    = $this->LanguageLookup($frame_language, false);

			$timestampindex = 0;
			$frame_remainingdata = substr($parsedFrame['data'], $frame_offset);
			while (strlen($frame_remainingdata)) {
				$frame_offset = 0;
				$frame_terminatorpos = strpos($frame_remainingdata, $this->TextEncodingTerminatorLookup($frame_textencoding));
				if ($frame_terminatorpos === false) {
					$frame_remainingdata = '';
				} else {
					if (ord(substr($frame_remainingdata, $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)), 1)) === 0) {
						$frame_terminatorpos++; // strpos() fooled because 2nd byte of Unicode chars are often 0x00
					}
					$parsedFrame['lyrics'][$timestampindex]['data'] = substr($frame_remainingdata, $frame_offset, $frame_terminatorpos - $frame_offset);

					$frame_remainingdata = substr($frame_remainingdata, $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)));
					if (($timestampindex == 0) && (ord($frame_remainingdata{0}) != 0)) {
						// timestamp probably omitted for first data item
					} else {
						$parsedFrame['lyrics'][$timestampindex]['timestamp'] = getid3_lib::BigEndian2Int(substr($frame_remainingdata, 0, 4));
						$frame_remainingdata = substr($frame_remainingdata, 4);
					}
					$timestampindex++;
				}
			}
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'COMM')) || // 4.10  COMM Comments
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'COM'))) {     // 4.11  COM  Comments
			//   There may be more than one comment frame in each tag,
			//   but only one with the same language and content descriptor.
			// <Header for 'Comment', ID: 'COMM'>
			// Text encoding          $xx
			// Language               $xx xx xx
			// Short content descrip. <text string according to encoding> $00 (00)
			// The actual text        <full text string according to encoding>

			if (strlen($parsedFrame['data']) < 5) {

				$ThisFileInfo['warning'][] = 'Invalid data (too short) for "'.$parsedFrame['frame_name'].'" frame at offset '.$parsedFrame['dataoffset'];

			} else {

				$frame_offset = 0;
				$frame_textencoding = ord(substr($parsedFrame['data'], $frame_offset++, 1));
				if ((($id3v2_majorversion <= 3) && ($frame_textencoding > 1)) || (($id3v2_majorversion == 4) && ($frame_textencoding > 3))) {
					$ThisFileInfo['warning'][] = 'Invalid text encoding byte ('.$frame_textencoding.') in frame "'.$parsedFrame['frame_name'].'" - defaulting to ISO-8859-1 encoding';
				}
				$frame_language = substr($parsedFrame['data'], $frame_offset, 3);
				$frame_offset += 3;
				$frame_terminatorpos = strpos($parsedFrame['data'], $this->TextEncodingTerminatorLookup($frame_textencoding), $frame_offset);
				if (ord(substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)), 1)) === 0) {
					$frame_terminatorpos++; // strpos() fooled because 2nd byte of Unicode chars are often 0x00
				}
				$frame_description = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
				if (ord($frame_description) === 0) {
					$frame_description = '';
				}
				$frame_text = (string) substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)));

				$parsedFrame['encodingid']   = $frame_textencoding;
				$parsedFrame['encoding']     = $this->TextEncodingNameLookup($frame_textencoding);

				$parsedFrame['language']     = $frame_language;
				$parsedFrame['languagename'] = $this->LanguageLookup($frame_language, false);
				$parsedFrame['description']  = $frame_description;
				$parsedFrame['data']         = $frame_text;
				if (!empty($parsedFrame['framenameshort']) && !empty($parsedFrame['data'])) {
					$ThisFileInfo['id3v2']['comments'][$parsedFrame['framenameshort']][] = getid3_lib::iconv_fallback($parsedFrame['encoding'], $ThisFileInfo['id3v2']['encoding'], $parsedFrame['data']);
				}

			}

		} elseif (($id3v2_majorversion >= 4) && ($parsedFrame['frame_name'] == 'RVA2')) { // 4.11  RVA2 Relative volume adjustment (2) (ID3v2.4+ only)
			//   There may be more than one 'RVA2' frame in each tag,
			//   but only one with the same identification string
			// <Header for 'Relative volume adjustment (2)', ID: 'RVA2'>
			// Identification          <text string> $00
			//   The 'identification' string is used to identify the situation and/or
			//   device where this adjustment should apply. The following is then
			//   repeated for every channel:
			// Type of channel         $xx
			// Volume adjustment       $xx xx
			// Bits representing peak  $xx
			// Peak volume             $xx (xx ...)

			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00");
			$frame_idstring = substr($parsedFrame['data'], 0, $frame_terminatorpos);
			if (ord($frame_idstring) === 0) {
				$frame_idstring = '';
			}
			$frame_remainingdata = substr($parsedFrame['data'], $frame_terminatorpos + strlen("\x00"));
			$parsedFrame['description'] = $frame_idstring;
			while (strlen($frame_remainingdata)) {
				$frame_offset = 0;
				$frame_channeltypeid = ord(substr($frame_remainingdata, $frame_offset++, 1));
				$parsedFrame[$frame_channeltypeid]['channeltypeid']  = $frame_channeltypeid;
				$parsedFrame[$frame_channeltypeid]['channeltype']    = $this->RVA2ChannelTypeLookup($frame_channeltypeid);
				$parsedFrame[$frame_channeltypeid]['volumeadjust']   = getid3_lib::BigEndian2Int(substr($frame_remainingdata, $frame_offset, 2), false, true); // 16-bit signed
				$frame_offset += 2;
				$parsedFrame[$frame_channeltypeid]['bitspeakvolume'] = ord(substr($frame_remainingdata, $frame_offset++, 1));
				$frame_bytespeakvolume = ceil($parsedFrame[$frame_channeltypeid]['bitspeakvolume'] / 8);
				$parsedFrame[$frame_channeltypeid]['peakvolume']     = getid3_lib::BigEndian2Int(substr($frame_remainingdata, $frame_offset, $frame_bytespeakvolume));
				$frame_remainingdata = substr($frame_remainingdata, $frame_offset + $frame_bytespeakvolume);
			}
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion == 3) && ($parsedFrame['frame_name'] == 'RVAD')) || // 4.12  RVAD Relative volume adjustment (ID3v2.3 only)
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'RVA'))) {     // 4.12  RVA  Relative volume adjustment (ID3v2.2 only)
			//   There may only be one 'RVA' frame in each tag
			// <Header for 'Relative volume adjustment', ID: 'RVA'>
			// ID3v2.2 => Increment/decrement     %000000ba
			// ID3v2.3 => Increment/decrement     %00fedcba
			// Bits used for volume descr.        $xx
			// Relative volume change, right      $xx xx (xx ...) // a
			// Relative volume change, left       $xx xx (xx ...) // b
			// Peak volume right                  $xx xx (xx ...)
			// Peak volume left                   $xx xx (xx ...)
			//   ID3v2.3 only, optional (not present in ID3v2.2):
			// Relative volume change, right back $xx xx (xx ...) // c
			// Relative volume change, left back  $xx xx (xx ...) // d
			// Peak volume right back             $xx xx (xx ...)
			// Peak volume left back              $xx xx (xx ...)
			//   ID3v2.3 only, optional (not present in ID3v2.2):
			// Relative volume change, center     $xx xx (xx ...) // e
			// Peak volume center                 $xx xx (xx ...)
			//   ID3v2.3 only, optional (not present in ID3v2.2):
			// Relative volume change, bass       $xx xx (xx ...) // f
			// Peak volume bass                   $xx xx (xx ...)

			$frame_offset = 0;
			$frame_incrdecrflags = getid3_lib::BigEndian2Bin(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['incdec']['right'] = (bool) substr($frame_incrdecrflags, 6, 1);
			$parsedFrame['incdec']['left']  = (bool) substr($frame_incrdecrflags, 7, 1);
			$parsedFrame['bitsvolume'] = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$frame_bytesvolume = ceil($parsedFrame['bitsvolume'] / 8);
			$parsedFrame['volumechange']['right'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, $frame_bytesvolume));
			if ($parsedFrame['incdec']['right'] === false) {
				$parsedFrame['volumechange']['right'] *= -1;
			}
			$frame_offset += $frame_bytesvolume;
			$parsedFrame['volumechange']['left'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, $frame_bytesvolume));
			if ($parsedFrame['incdec']['left'] === false) {
				$parsedFrame['volumechange']['left'] *= -1;
			}
			$frame_offset += $frame_bytesvolume;
			$parsedFrame['peakvolume']['right'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, $frame_bytesvolume));
			$frame_offset += $frame_bytesvolume;
			$parsedFrame['peakvolume']['left']  = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, $frame_bytesvolume));
			$frame_offset += $frame_bytesvolume;
			if ($id3v2_majorversion == 3) {
				$parsedFrame['data'] = substr($parsedFrame['data'], $frame_offset);
				if (strlen($parsedFrame['data']) > 0) {
					$parsedFrame['incdec']['rightrear'] = (bool) substr($frame_incrdecrflags, 4, 1);
					$parsedFrame['incdec']['leftrear']  = (bool) substr($frame_incrdecrflags, 5, 1);
					$parsedFrame['volumechange']['rightrear'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, $frame_bytesvolume));
					if ($parsedFrame['incdec']['rightrear'] === false) {
						$parsedFrame['volumechange']['rightrear'] *= -1;
					}
					$frame_offset += $frame_bytesvolume;
					$parsedFrame['volumechange']['leftrear'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, $frame_bytesvolume));
					if ($parsedFrame['incdec']['leftrear'] === false) {
						$parsedFrame['volumechange']['leftrear'] *= -1;
					}
					$frame_offset += $frame_bytesvolume;
					$parsedFrame['peakvolume']['rightrear'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, $frame_bytesvolume));
					$frame_offset += $frame_bytesvolume;
					$parsedFrame['peakvolume']['leftrear']  = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, $frame_bytesvolume));
					$frame_offset += $frame_bytesvolume;
				}
				$parsedFrame['data'] = substr($parsedFrame['data'], $frame_offset);
				if (strlen($parsedFrame['data']) > 0) {
					$parsedFrame['incdec']['center'] = (bool) substr($frame_incrdecrflags, 3, 1);
					$parsedFrame['volumechange']['center'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, $frame_bytesvolume));
					if ($parsedFrame['incdec']['center'] === false) {
						$parsedFrame['volumechange']['center'] *= -1;
					}
					$frame_offset += $frame_bytesvolume;
					$parsedFrame['peakvolume']['center'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, $frame_bytesvolume));
					$frame_offset += $frame_bytesvolume;
				}
				$parsedFrame['data'] = substr($parsedFrame['data'], $frame_offset);
				if (strlen($parsedFrame['data']) > 0) {
					$parsedFrame['incdec']['bass'] = (bool) substr($frame_incrdecrflags, 2, 1);
					$parsedFrame['volumechange']['bass'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, $frame_bytesvolume));
					if ($parsedFrame['incdec']['bass'] === false) {
						$parsedFrame['volumechange']['bass'] *= -1;
					}
					$frame_offset += $frame_bytesvolume;
					$parsedFrame['peakvolume']['bass'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, $frame_bytesvolume));
					$frame_offset += $frame_bytesvolume;
				}
			}
			unset($parsedFrame['data']);


		} elseif (($id3v2_majorversion >= 4) && ($parsedFrame['frame_name'] == 'EQU2')) { // 4.12  EQU2 Equalisation (2) (ID3v2.4+ only)
			//   There may be more than one 'EQU2' frame in each tag,
			//   but only one with the same identification string
			// <Header of 'Equalisation (2)', ID: 'EQU2'>
			// Interpolation method  $xx
			//   $00  Band
			//   $01  Linear
			// Identification        <text string> $00
			//   The following is then repeated for every adjustment point
			// Frequency          $xx xx
			// Volume adjustment  $xx xx

			$frame_offset = 0;
			$frame_interpolationmethod = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_idstring = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_idstring) === 0) {
				$frame_idstring = '';
			}
			$parsedFrame['description'] = $frame_idstring;
			$frame_remainingdata = substr($parsedFrame['data'], $frame_terminatorpos + strlen("\x00"));
			while (strlen($frame_remainingdata)) {
				$frame_frequency = getid3_lib::BigEndian2Int(substr($frame_remainingdata, 0, 2)) / 2;
				$parsedFrame['data'][$frame_frequency] = getid3_lib::BigEndian2Int(substr($frame_remainingdata, 2, 2), false, true);
				$frame_remainingdata = substr($frame_remainingdata, 4);
			}
			$parsedFrame['interpolationmethod'] = $frame_interpolationmethod;
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion == 3) && ($parsedFrame['frame_name'] == 'EQUA')) || // 4.12  EQUA Equalisation (ID3v2.3 only)
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'EQU'))) {     // 4.13  EQU  Equalisation (ID3v2.2 only)
			//   There may only be one 'EQUA' frame in each tag
			// <Header for 'Relative volume adjustment', ID: 'EQU'>
			// Adjustment bits    $xx
			//   This is followed by 2 bytes + ('adjustment bits' rounded up to the
			//   nearest byte) for every equalisation band in the following format,
			//   giving a frequency range of 0 - 32767Hz:
			// Increment/decrement   %x (MSB of the Frequency)
			// Frequency             (lower 15 bits)
			// Adjustment            $xx (xx ...)

			$frame_offset = 0;
			$parsedFrame['adjustmentbits'] = substr($parsedFrame['data'], $frame_offset++, 1);
			$frame_adjustmentbytes = ceil($parsedFrame['adjustmentbits'] / 8);

			$frame_remainingdata = (string) substr($parsedFrame['data'], $frame_offset);
			while (strlen($frame_remainingdata) > 0) {
				$frame_frequencystr = getid3_lib::BigEndian2Bin(substr($frame_remainingdata, 0, 2));
				$frame_incdec    = (bool) substr($frame_frequencystr, 0, 1);
				$frame_frequency = bindec(substr($frame_frequencystr, 1, 15));
				$parsedFrame[$frame_frequency]['incdec'] = $frame_incdec;
				$parsedFrame[$frame_frequency]['adjustment'] = getid3_lib::BigEndian2Int(substr($frame_remainingdata, 2, $frame_adjustmentbytes));
				if ($parsedFrame[$frame_frequency]['incdec'] === false) {
					$parsedFrame[$frame_frequency]['adjustment'] *= -1;
				}
				$frame_remainingdata = substr($frame_remainingdata, 2 + $frame_adjustmentbytes);
			}
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'RVRB')) || // 4.13  RVRB Reverb
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'REV'))) {     // 4.14  REV  Reverb
			//   There may only be one 'RVRB' frame in each tag.
			// <Header for 'Reverb', ID: 'RVRB'>
			// Reverb left (ms)                 $xx xx
			// Reverb right (ms)                $xx xx
			// Reverb bounces, left             $xx
			// Reverb bounces, right            $xx
			// Reverb feedback, left to left    $xx
			// Reverb feedback, left to right   $xx
			// Reverb feedback, right to right  $xx
			// Reverb feedback, right to left   $xx
			// Premix left to right             $xx
			// Premix right to left             $xx

			$frame_offset = 0;
			$parsedFrame['left']  = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, 2));
			$frame_offset += 2;
			$parsedFrame['right'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, 2));
			$frame_offset += 2;
			$parsedFrame['bouncesL']      = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['bouncesR']      = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['feedbackLL']    = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['feedbackLR']    = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['feedbackRR']    = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['feedbackRL']    = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['premixLR']      = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['premixRL']      = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'APIC')) || // 4.14  APIC Attached picture
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'PIC'))) {     // 4.15  PIC  Attached picture
			//   There may be several pictures attached to one file,
			//   each in their individual 'APIC' frame, but only one
			//   with the same content descriptor
			// <Header for 'Attached picture', ID: 'APIC'>
			// Text encoding      $xx
			// ID3v2.3+ => MIME type          <text string> $00
			// ID3v2.2  => Image format       $xx xx xx
			// Picture type       $xx
			// Description        <text string according to encoding> $00 (00)
			// Picture data       <binary data>

			$frame_offset = 0;
			$frame_textencoding = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			if ((($id3v2_majorversion <= 3) && ($frame_textencoding > 1)) || (($id3v2_majorversion == 4) && ($frame_textencoding > 3))) {
				$ThisFileInfo['warning'][] = 'Invalid text encoding byte ('.$frame_textencoding.') in frame "'.$parsedFrame['frame_name'].'" - defaulting to ISO-8859-1 encoding';
			}

			if ($id3v2_majorversion == 2) {
				$frame_imagetype = substr($parsedFrame['data'], $frame_offset, 3);
				if (strtolower($frame_imagetype) == 'ima') {
					// complete hack for mp3Rage (www.chaoticsoftware.com) that puts ID3v2.3-formatted
					// MIME type instead of 3-char ID3v2.2-format image type  (thanks xbhoffØpacbell*net)
					$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
					$frame_mimetype = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
					if (ord($frame_mimetype) === 0) {
						$frame_mimetype = '';
					}
					$frame_imagetype = strtoupper(str_replace('image/', '', strtolower($frame_mimetype)));
					if ($frame_imagetype == 'JPEG') {
						$frame_imagetype = 'JPG';
					}
					$frame_offset = $frame_terminatorpos + strlen("\x00");
				} else {
					$frame_offset += 3;
				}
			}
			if ($id3v2_majorversion > 2) {
				$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
				$frame_mimetype = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
				if (ord($frame_mimetype) === 0) {
					$frame_mimetype = '';
				}
				$frame_offset = $frame_terminatorpos + strlen("\x00");
			}

			$frame_picturetype = ord(substr($parsedFrame['data'], $frame_offset++, 1));

			$frame_terminatorpos = strpos($parsedFrame['data'], $this->TextEncodingTerminatorLookup($frame_textencoding), $frame_offset);
			if (ord(substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)), 1)) === 0) {
				$frame_terminatorpos++; // strpos() fooled because 2nd byte of Unicode chars are often 0x00
			}
			$frame_description = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_description) === 0) {
				$frame_description = '';
			}
			$parsedFrame['encodingid']       = $frame_textencoding;
			$parsedFrame['encoding']         = $this->TextEncodingNameLookup($frame_textencoding);

			if ($id3v2_majorversion == 2) {
				$parsedFrame['imagetype']    = $frame_imagetype;
			} else {
				$parsedFrame['mime']         = $frame_mimetype;
			}
			$parsedFrame['picturetypeid']    = $frame_picturetype;
			$parsedFrame['picturetype']      = $this->APICPictureTypeLookup($frame_picturetype);
			$parsedFrame['description']      = $frame_description;
			$parsedFrame['data']             = substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)));

			$imagechunkcheck = getid3_lib::GetDataImageSize($parsedFrame['data']);
			if (($imagechunkcheck[2] >= 1) && ($imagechunkcheck[2] <= 3)) {
				$parsedFrame['image_mime']       = 'image/'.getid3_lib::ImageTypesLookup($imagechunkcheck[2]);
				if ($imagechunkcheck[0]) {
					$parsedFrame['image_width']  = $imagechunkcheck[0];
				}
				if ($imagechunkcheck[1]) {
					$parsedFrame['image_height'] = $imagechunkcheck[1];
				}
				$parsedFrame['image_bytes']      = strlen($parsedFrame['data']);
			}


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'GEOB')) || // 4.15  GEOB General encapsulated object
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'GEO'))) {     // 4.16  GEO  General encapsulated object
			//   There may be more than one 'GEOB' frame in each tag,
			//   but only one with the same content descriptor
			// <Header for 'General encapsulated object', ID: 'GEOB'>
			// Text encoding          $xx
			// MIME type              <text string> $00
			// Filename               <text string according to encoding> $00 (00)
			// Content description    <text string according to encoding> $00 (00)
			// Encapsulated object    <binary data>

			$frame_offset = 0;
			$frame_textencoding = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			if ((($id3v2_majorversion <= 3) && ($frame_textencoding > 1)) || (($id3v2_majorversion == 4) && ($frame_textencoding > 3))) {
				$ThisFileInfo['warning'][] = 'Invalid text encoding byte ('.$frame_textencoding.') in frame "'.$parsedFrame['frame_name'].'" - defaulting to ISO-8859-1 encoding';
			}
			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_mimetype = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_mimetype) === 0) {
				$frame_mimetype = '';
			}
			$frame_offset = $frame_terminatorpos + strlen("\x00");

			$frame_terminatorpos = strpos($parsedFrame['data'], $this->TextEncodingTerminatorLookup($frame_textencoding), $frame_offset);
			if (ord(substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)), 1)) === 0) {
				$frame_terminatorpos++; // strpos() fooled because 2nd byte of Unicode chars are often 0x00
			}
			$frame_filename = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_filename) === 0) {
				$frame_filename = '';
			}
			$frame_offset = $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding));

			$frame_terminatorpos = strpos($parsedFrame['data'], $this->TextEncodingTerminatorLookup($frame_textencoding), $frame_offset);
			if (ord(substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)), 1)) === 0) {
				$frame_terminatorpos++; // strpos() fooled because 2nd byte of Unicode chars are often 0x00
			}
			$frame_description = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_description) === 0) {
				$frame_description = '';
			}
			$frame_offset = $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding));

			$parsedFrame['objectdata']  = (string) substr($parsedFrame['data'], $frame_offset);
			$parsedFrame['encodingid']  = $frame_textencoding;
			$parsedFrame['encoding']    = $this->TextEncodingNameLookup($frame_textencoding);

			$parsedFrame['mime']        = $frame_mimetype;
			$parsedFrame['filename']    = $frame_filename;
			$parsedFrame['description'] = $frame_description;
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'PCNT')) || // 4.16  PCNT Play counter
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'CNT'))) {     // 4.17  CNT  Play counter
			//   There may only be one 'PCNT' frame in each tag.
			//   When the counter reaches all one's, one byte is inserted in
			//   front of the counter thus making the counter eight bits bigger
			// <Header for 'Play counter', ID: 'PCNT'>
			// Counter        $xx xx xx xx (xx ...)

			$parsedFrame['data']          = getid3_lib::BigEndian2Int($parsedFrame['data']);


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'POPM')) || // 4.17  POPM Popularimeter
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'POP'))) {     // 4.18  POP  Popularimeter
			//   There may be more than one 'POPM' frame in each tag,
			//   but only one with the same email address
			// <Header for 'Popularimeter', ID: 'POPM'>
			// Email to user   <text string> $00
			// Rating          $xx
			// Counter         $xx xx xx xx (xx ...)

			$frame_offset = 0;
			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_emailaddress = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_emailaddress) === 0) {
				$frame_emailaddress = '';
			}
			$frame_offset = $frame_terminatorpos + strlen("\x00");
			$frame_rating = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['data'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset));
			$parsedFrame['email']  = $frame_emailaddress;
			$parsedFrame['rating'] = $frame_rating;
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'RBUF')) || // 4.18  RBUF Recommended buffer size
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'BUF'))) {     // 4.19  BUF  Recommended buffer size
			//   There may only be one 'RBUF' frame in each tag
			// <Header for 'Recommended buffer size', ID: 'RBUF'>
			// Buffer size               $xx xx xx
			// Embedded info flag        %0000000x
			// Offset to next tag        $xx xx xx xx

			$frame_offset = 0;
			$parsedFrame['buffersize'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, 3));
			$frame_offset += 3;

			$frame_embeddedinfoflags = getid3_lib::BigEndian2Bin(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['flags']['embededinfo'] = (bool) substr($frame_embeddedinfoflags, 7, 1);
			$parsedFrame['nexttagoffset'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, 4));
			unset($parsedFrame['data']);


		} elseif (($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'CRM')) { // 4.20  Encrypted meta frame (ID3v2.2 only)
			//   There may be more than one 'CRM' frame in a tag,
			//   but only one with the same 'owner identifier'
			// <Header for 'Encrypted meta frame', ID: 'CRM'>
			// Owner identifier      <textstring> $00 (00)
			// Content/explanation   <textstring> $00 (00)
			// Encrypted datablock   <binary data>

			$frame_offset = 0;
			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_ownerid = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			$frame_offset = $frame_terminatorpos + strlen("\x00");

			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_description = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_description) === 0) {
				$frame_description = '';
			}
			$frame_offset = $frame_terminatorpos + strlen("\x00");

			$parsedFrame['ownerid']     = $frame_ownerid;
			$parsedFrame['data']        = (string) substr($parsedFrame['data'], $frame_offset);
			$parsedFrame['description'] = $frame_description;
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'AENC')) || // 4.19  AENC Audio encryption
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'CRA'))) {     // 4.21  CRA  Audio encryption
			//   There may be more than one 'AENC' frames in a tag,
			//   but only one with the same 'Owner identifier'
			// <Header for 'Audio encryption', ID: 'AENC'>
			// Owner identifier   <text string> $00
			// Preview start      $xx xx
			// Preview length     $xx xx
			// Encryption info    <binary data>

			$frame_offset = 0;
			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_ownerid = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_ownerid) === 0) {
				$frame_ownerid == '';
			}
			$frame_offset = $frame_terminatorpos + strlen("\x00");
			$parsedFrame['ownerid'] = $frame_ownerid;
			$parsedFrame['previewstart'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, 2));
			$frame_offset += 2;
			$parsedFrame['previewlength'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, 2));
			$frame_offset += 2;
			$parsedFrame['encryptioninfo'] = (string) substr($parsedFrame['data'], $frame_offset);
			unset($parsedFrame['data']);


		} elseif ((($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'LINK')) || // 4.20  LINK Linked information
				(($id3v2_majorversion == 2) && ($parsedFrame['frame_name'] == 'LNK'))) {     // 4.22  LNK  Linked information
			//   There may be more than one 'LINK' frame in a tag,
			//   but only one with the same contents
			// <Header for 'Linked information', ID: 'LINK'>
			// ID3v2.3+ => Frame identifier   $xx xx xx xx
			// ID3v2.2  => Frame identifier   $xx xx xx
			// URL                            <text string> $00
			// ID and additional data         <text string(s)>

			$frame_offset = 0;
			if ($id3v2_majorversion == 2) {
				$parsedFrame['frameid'] = substr($parsedFrame['data'], $frame_offset, 3);
				$frame_offset += 3;
			} else {
				$parsedFrame['frameid'] = substr($parsedFrame['data'], $frame_offset, 4);
				$frame_offset += 4;
			}

			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_url = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_url) === 0) {
				$frame_url = '';
			}
			$frame_offset = $frame_terminatorpos + strlen("\x00");
			$parsedFrame['url'] = $frame_url;

			$parsedFrame['additionaldata'] = (string) substr($parsedFrame['data'], $frame_offset);
			if (!empty($parsedFrame['framenameshort']) && $parsedFrame['url']) {
				$ThisFileInfo['id3v2']['comments'][$parsedFrame['framenameshort']][] = getid3_lib::iconv_fallback($parsedFrame['encoding'], $ThisFileInfo['id3v2']['encoding'], $parsedFrame['url']);
			}
			unset($parsedFrame['data']);


		} elseif (($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'POSS')) { // 4.21  POSS Position synchronisation frame (ID3v2.3+ only)
			//   There may only be one 'POSS' frame in each tag
			// <Head for 'Position synchronisation', ID: 'POSS'>
			// Time stamp format         $xx
			// Position                  $xx (xx ...)

			$frame_offset = 0;
			$parsedFrame['timestampformat'] = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['position']        = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset));
			unset($parsedFrame['data']);


		} elseif (($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'USER')) { // 4.22  USER Terms of use (ID3v2.3+ only)
			//   There may be more than one 'Terms of use' frame in a tag,
			//   but only one with the same 'Language'
			// <Header for 'Terms of use frame', ID: 'USER'>
			// Text encoding        $xx
			// Language             $xx xx xx
			// The actual text      <text string according to encoding>

			$frame_offset = 0;
			$frame_textencoding = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			if ((($id3v2_majorversion <= 3) && ($frame_textencoding > 1)) || (($id3v2_majorversion == 4) && ($frame_textencoding > 3))) {
				$ThisFileInfo['warning'][] = 'Invalid text encoding byte ('.$frame_textencoding.') in frame "'.$parsedFrame['frame_name'].'" - defaulting to ISO-8859-1 encoding';
			}
			$frame_language = substr($parsedFrame['data'], $frame_offset, 3);
			$frame_offset += 3;
			$parsedFrame['language']     = $frame_language;
			$parsedFrame['languagename'] = $this->LanguageLookup($frame_language, false);
			$parsedFrame['encodingid']   = $frame_textencoding;
			$parsedFrame['encoding']     = $this->TextEncodingNameLookup($frame_textencoding);

			$parsedFrame['data']         = (string) substr($parsedFrame['data'], $frame_offset);
			if (!empty($parsedFrame['framenameshort']) && !empty($parsedFrame['data'])) {
				$ThisFileInfo['id3v2']['comments'][$parsedFrame['framenameshort']][] = getid3_lib::iconv_fallback($parsedFrame['encoding'], $ThisFileInfo['id3v2']['encoding'], $parsedFrame['data']);
			}
			unset($parsedFrame['data']);


		} elseif (($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'OWNE')) { // 4.23  OWNE Ownership frame (ID3v2.3+ only)
			//   There may only be one 'OWNE' frame in a tag
			// <Header for 'Ownership frame', ID: 'OWNE'>
			// Text encoding     $xx
			// Price paid        <text string> $00
			// Date of purch.    <text string>
			// Seller            <text string according to encoding>

			$frame_offset = 0;
			$frame_textencoding = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			if ((($id3v2_majorversion <= 3) && ($frame_textencoding > 1)) || (($id3v2_majorversion == 4) && ($frame_textencoding > 3))) {
				$ThisFileInfo['warning'][] = 'Invalid text encoding byte ('.$frame_textencoding.') in frame "'.$parsedFrame['frame_name'].'" - defaulting to ISO-8859-1 encoding';
			}
			$parsedFrame['encodingid'] = $frame_textencoding;
			$parsedFrame['encoding']   = $this->TextEncodingNameLookup($frame_textencoding);

			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_pricepaid = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			$frame_offset = $frame_terminatorpos + strlen("\x00");

			$parsedFrame['pricepaid']['currencyid'] = substr($frame_pricepaid, 0, 3);
			$parsedFrame['pricepaid']['currency']   = $this->LookupCurrencyUnits($parsedFrame['pricepaid']['currencyid']);
			$parsedFrame['pricepaid']['value']      = substr($frame_pricepaid, 3);

			$parsedFrame['purchasedate'] = substr($parsedFrame['data'], $frame_offset, 8);
			if (!$this->IsValidDateStampString($parsedFrame['purchasedate'])) {
				$parsedFrame['purchasedateunix'] = mktime (0, 0, 0, substr($parsedFrame['purchasedate'], 4, 2), substr($parsedFrame['purchasedate'], 6, 2), substr($parsedFrame['purchasedate'], 0, 4));
			}
			$frame_offset += 8;

			$parsedFrame['seller'] = (string) substr($parsedFrame['data'], $frame_offset);
			unset($parsedFrame['data']);


		} elseif (($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'COMR')) { // 4.24  COMR Commercial frame (ID3v2.3+ only)
			//   There may be more than one 'commercial frame' in a tag,
			//   but no two may be identical
			// <Header for 'Commercial frame', ID: 'COMR'>
			// Text encoding      $xx
			// Price string       <text string> $00
			// Valid until        <text string>
			// Contact URL        <text string> $00
			// Received as        $xx
			// Name of seller     <text string according to encoding> $00 (00)
			// Description        <text string according to encoding> $00 (00)
			// Picture MIME type  <string> $00
			// Seller logo        <binary data>

			$frame_offset = 0;
			$frame_textencoding = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			if ((($id3v2_majorversion <= 3) && ($frame_textencoding > 1)) || (($id3v2_majorversion == 4) && ($frame_textencoding > 3))) {
				$ThisFileInfo['warning'][] = 'Invalid text encoding byte ('.$frame_textencoding.') in frame "'.$parsedFrame['frame_name'].'" - defaulting to ISO-8859-1 encoding';
			}

			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_pricestring = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			$frame_offset = $frame_terminatorpos + strlen("\x00");
			$frame_rawpricearray = explode('/', $frame_pricestring);
			foreach ($frame_rawpricearray as $key => $val) {
				$frame_currencyid = substr($val, 0, 3);
				$parsedFrame['price'][$frame_currencyid]['currency'] = $this->LookupCurrencyUnits($frame_currencyid);
				$parsedFrame['price'][$frame_currencyid]['value']    = substr($val, 3);
			}

			$frame_datestring = substr($parsedFrame['data'], $frame_offset, 8);
			$frame_offset += 8;

			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_contacturl = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			$frame_offset = $frame_terminatorpos + strlen("\x00");

			$frame_receivedasid = ord(substr($parsedFrame['data'], $frame_offset++, 1));

			$frame_terminatorpos = strpos($parsedFrame['data'], $this->TextEncodingTerminatorLookup($frame_textencoding), $frame_offset);
			if (ord(substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)), 1)) === 0) {
				$frame_terminatorpos++; // strpos() fooled because 2nd byte of Unicode chars are often 0x00
			}
			$frame_sellername = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_sellername) === 0) {
				$frame_sellername = '';
			}
			$frame_offset = $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding));

			$frame_terminatorpos = strpos($parsedFrame['data'], $this->TextEncodingTerminatorLookup($frame_textencoding), $frame_offset);
			if (ord(substr($parsedFrame['data'], $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding)), 1)) === 0) {
				$frame_terminatorpos++; // strpos() fooled because 2nd byte of Unicode chars are often 0x00
			}
			$frame_description = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_description) === 0) {
				$frame_description = '';
			}
			$frame_offset = $frame_terminatorpos + strlen($this->TextEncodingTerminatorLookup($frame_textencoding));

			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_mimetype = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			$frame_offset = $frame_terminatorpos + strlen("\x00");

			$frame_sellerlogo = substr($parsedFrame['data'], $frame_offset);

			$parsedFrame['encodingid']        = $frame_textencoding;
			$parsedFrame['encoding']          = $this->TextEncodingNameLookup($frame_textencoding);

			$parsedFrame['pricevaliduntil']   = $frame_datestring;
			$parsedFrame['contacturl']        = $frame_contacturl;
			$parsedFrame['receivedasid']      = $frame_receivedasid;
			$parsedFrame['receivedas']        = $this->COMRReceivedAsLookup($frame_receivedasid);
			$parsedFrame['sellername']        = $frame_sellername;
			$parsedFrame['description']       = $frame_description;
			$parsedFrame['mime']              = $frame_mimetype;
			$parsedFrame['logo']              = $frame_sellerlogo;
			unset($parsedFrame['data']);


		} elseif (($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'ENCR')) { // 4.25  ENCR Encryption method registration (ID3v2.3+ only)
			//   There may be several 'ENCR' frames in a tag,
			//   but only one containing the same symbol
			//   and only one containing the same owner identifier
			// <Header for 'Encryption method registration', ID: 'ENCR'>
			// Owner identifier    <text string> $00
			// Method symbol       $xx
			// Encryption data     <binary data>

			$frame_offset = 0;
			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_ownerid = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_ownerid) === 0) {
				$frame_ownerid = '';
			}
			$frame_offset = $frame_terminatorpos + strlen("\x00");

			$parsedFrame['ownerid']      = $frame_ownerid;
			$parsedFrame['methodsymbol'] = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['data']         = (string) substr($parsedFrame['data'], $frame_offset);


		} elseif (($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'GRID')) { // 4.26  GRID Group identification registration (ID3v2.3+ only)

			//   There may be several 'GRID' frames in a tag,
			//   but only one containing the same symbol
			//   and only one containing the same owner identifier
			// <Header for 'Group ID registration', ID: 'GRID'>
			// Owner identifier      <text string> $00
			// Group symbol          $xx
			// Group dependent data  <binary data>

			$frame_offset = 0;
			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_ownerid = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_ownerid) === 0) {
				$frame_ownerid = '';
			}
			$frame_offset = $frame_terminatorpos + strlen("\x00");

			$parsedFrame['ownerid']       = $frame_ownerid;
			$parsedFrame['groupsymbol']   = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['data']          = (string) substr($parsedFrame['data'], $frame_offset);


		} elseif (($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'PRIV')) { // 4.27  PRIV Private frame (ID3v2.3+ only)
			//   The tag may contain more than one 'PRIV' frame
			//   but only with different contents
			// <Header for 'Private frame', ID: 'PRIV'>
			// Owner identifier      <text string> $00
			// The private data      <binary data>

			$frame_offset = 0;
			$frame_terminatorpos = strpos($parsedFrame['data'], "\x00", $frame_offset);
			$frame_ownerid = substr($parsedFrame['data'], $frame_offset, $frame_terminatorpos - $frame_offset);
			if (ord($frame_ownerid) === 0) {
				$frame_ownerid = '';
			}
			$frame_offset = $frame_terminatorpos + strlen("\x00");

			$parsedFrame['ownerid'] = $frame_ownerid;
			$parsedFrame['data']    = (string) substr($parsedFrame['data'], $frame_offset);


		} elseif (($id3v2_majorversion >= 4) && ($parsedFrame['frame_name'] == 'SIGN')) { // 4.28  SIGN Signature frame (ID3v2.4+ only)
			//   There may be more than one 'signature frame' in a tag,
			//   but no two may be identical
			// <Header for 'Signature frame', ID: 'SIGN'>
			// Group symbol      $xx
			// Signature         <binary data>

			$frame_offset = 0;
			$parsedFrame['groupsymbol'] = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$parsedFrame['data']        = (string) substr($parsedFrame['data'], $frame_offset);


		} elseif (($id3v2_majorversion >= 4) && ($parsedFrame['frame_name'] == 'SEEK')) { // 4.29  SEEK Seek frame (ID3v2.4+ only)
			//   There may only be one 'seek frame' in a tag
			// <Header for 'Seek frame', ID: 'SEEK'>
			// Minimum offset to next tag       $xx xx xx xx

			$frame_offset = 0;
			$parsedFrame['data']          = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, 4));


		} elseif (($id3v2_majorversion >= 4) && ($parsedFrame['frame_name'] == 'ASPI')) { // 4.30  ASPI Audio seek point index (ID3v2.4+ only)
			//   There may only be one 'audio seek point index' frame in a tag
			// <Header for 'Seek Point Index', ID: 'ASPI'>
			// Indexed data start (S)         $xx xx xx xx
			// Indexed data length (L)        $xx xx xx xx
			// Number of index points (N)     $xx xx
			// Bits per index point (b)       $xx
			//   Then for every index point the following data is included:
			// Fraction at index (Fi)          $xx (xx)

			$frame_offset = 0;
			$parsedFrame['datastart'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, 4));
			$frame_offset += 4;
			$parsedFrame['indexeddatalength'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, 4));
			$frame_offset += 4;
			$parsedFrame['indexpoints'] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, 2));
			$frame_offset += 2;
			$parsedFrame['bitsperpoint'] = ord(substr($parsedFrame['data'], $frame_offset++, 1));
			$frame_bytesperpoint = ceil($parsedFrame['bitsperpoint'] / 8);
			for ($i = 0; $i < $frame_indexpoints; $i++) {
				$parsedFrame['indexes'][$i] = getid3_lib::BigEndian2Int(substr($parsedFrame['data'], $frame_offset, $frame_bytesperpoint));
				$frame_offset += $frame_bytesperpoint;
			}
			unset($parsedFrame['data']);

		} elseif (($id3v2_majorversion >= 3) && ($parsedFrame['frame_name'] == 'RGAD')) { // Replay Gain Adjustment
			// http://privatewww.essex.ac.uk/~djmrob/replaygain/file_format_id3v2.html
			//   There may only be one 'RGAD' frame in a tag
			// <Header for 'Replay Gain Adjustment', ID: 'RGAD'>
			// Peak Amplitude                      $xx $xx $xx $xx
			// Radio Replay Gain Adjustment        %aaabbbcd %dddddddd
			// Audiophile Replay Gain Adjustment   %aaabbbcd %dddddddd
			//   a - name code
			//   b - originator code
			//   c - sign bit
			//   d - replay gain adjustment

			$frame_offset = 0;
			$parsedFrame['peakamplitude'] = getid3_lib::BigEndian2Float(substr($parsedFrame['data'], $frame_offset, 4));
			$frame_offset += 4;
			$rg_track_adjustment = getid3_lib::Dec2Bin(substr($parsedFrame['data'], $frame_offset, 2));
			$frame_offset += 2;
			$rg_album_adjustment = getid3_lib::Dec2Bin(substr($parsedFrame['data'], $frame_offset, 2));
			$frame_offset += 2;
			$parsedFrame['raw']['track']['name']       = getid3_lib::Bin2Dec(substr($rg_track_adjustment, 0, 3));
			$parsedFrame['raw']['track']['originator'] = getid3_lib::Bin2Dec(substr($rg_track_adjustment, 3, 3));
			$parsedFrame['raw']['track']['signbit']    = getid3_lib::Bin2Dec(substr($rg_track_adjustment, 6, 1));
			$parsedFrame['raw']['track']['adjustment'] = getid3_lib::Bin2Dec(substr($rg_track_adjustment, 7, 9));
			$parsedFrame['raw']['album']['name']       = getid3_lib::Bin2Dec(substr($rg_album_adjustment, 0, 3));
			$parsedFrame['raw']['album']['originator'] = getid3_lib::Bin2Dec(substr($rg_album_adjustment, 3, 3));
			$parsedFrame['raw']['album']['signbit']    = getid3_lib::Bin2Dec(substr($rg_album_adjustment, 6, 1));
			$parsedFrame['raw']['album']['adjustment'] = getid3_lib::Bin2Dec(substr($rg_album_adjustment, 7, 9));
			$parsedFrame['track']['name']       = getid3_lib::RGADnameLookup($parsedFrame['raw']['track']['name']);
			$parsedFrame['track']['originator'] = getid3_lib::RGADoriginatorLookup($parsedFrame['raw']['track']['originator']);
			$parsedFrame['track']['adjustment'] = getid3_lib::RGADadjustmentLookup($parsedFrame['raw']['track']['adjustment'], $parsedFrame['raw']['track']['signbit']);
			$parsedFrame['album']['name']       = getid3_lib::RGADnameLookup($parsedFrame['raw']['album']['name']);
			$parsedFrame['album']['originator'] = getid3_lib::RGADoriginatorLookup($parsedFrame['raw']['album']['originator']);
			$parsedFrame['album']['adjustment'] = getid3_lib::RGADadjustmentLookup($parsedFrame['raw']['album']['adjustment'], $parsedFrame['raw']['album']['signbit']);

			$ThisFileInfo['replay_gain']['track']['peak']       = $parsedFrame['peakamplitude'];
			$ThisFileInfo['replay_gain']['track']['originator'] = $parsedFrame['track']['originator'];
			$ThisFileInfo['replay_gain']['track']['adjustment'] = $parsedFrame['track']['adjustment'];
			$ThisFileInfo['replay_gain']['album']['originator'] = $parsedFrame['album']['originator'];
			$ThisFileInfo['replay_gain']['album']['adjustment'] = $parsedFrame['album']['adjustment'];

			unset($parsedFrame['data']);

		}

		return true;
	}


	function DeUnsynchronise($data) {
		return str_replace("\xFF\x00", "\xFF", $data);
	}

	function LookupCurrencyUnits($currencyid) {

		$begin = __LINE__;

		/** This is not a comment!


			AED	Dirhams
			AFA	Afghanis
			ALL	Leke
			AMD	Drams
			ANG	Guilders
			AOA	Kwanza
			ARS	Pesos
			ATS	Schillings
			AUD	Dollars
			AWG	Guilders
			AZM	Manats
			BAM	Convertible Marka
			BBD	Dollars
			BDT	Taka
			BEF	Francs
			BGL	Leva
			BHD	Dinars
			BIF	Francs
			BMD	Dollars
			BND	Dollars
			BOB	Bolivianos
			BRL	Brazil Real
			BSD	Dollars
			BTN	Ngultrum
			BWP	Pulas
			BYR	Rubles
			BZD	Dollars
			CAD	Dollars
			CDF	Congolese Francs
			CHF	Francs
			CLP	Pesos
			CNY	Yuan Renminbi
			COP	Pesos
			CRC	Colones
			CUP	Pesos
			CVE	Escudos
			CYP	Pounds
			CZK	Koruny
			DEM	Deutsche Marks
			DJF	Francs
			DKK	Kroner
			DOP	Pesos
			DZD	Algeria Dinars
			EEK	Krooni
			EGP	Pounds
			ERN	Nakfa
			ESP	Pesetas
			ETB	Birr
			EUR	Euro
			FIM	Markkaa
			FJD	Dollars
			FKP	Pounds
			FRF	Francs
			GBP	Pounds
			GEL	Lari
			GGP	Pounds
			GHC	Cedis
			GIP	Pounds
			GMD	Dalasi
			GNF	Francs
			GRD	Drachmae
			GTQ	Quetzales
			GYD	Dollars
			HKD	Dollars
			HNL	Lempiras
			HRK	Kuna
			HTG	Gourdes
			HUF	Forints
			IDR	Rupiahs
			IEP	Pounds
			ILS	New Shekels
			IMP	Pounds
			INR	Rupees
			IQD	Dinars
			IRR	Rials
			ISK	Kronur
			ITL	Lire
			JEP	Pounds
			JMD	Dollars
			JOD	Dinars
			JPY	Yen
			KES	Shillings
			KGS	Soms
			KHR	Riels
			KMF	Francs
			KPW	Won
			KWD	Dinars
			KYD	Dollars
			KZT	Tenge
			LAK	Kips
			LBP	Pounds
			LKR	Rupees
			LRD	Dollars
			LSL	Maloti
			LTL	Litai
			LUF	Francs
			LVL	Lati
			LYD	Dinars
			MAD	Dirhams
			MDL	Lei
			MGF	Malagasy Francs
			MKD	Denars
			MMK	Kyats
			MNT	Tugriks
			MOP	Patacas
			MRO	Ouguiyas
			MTL	Liri
			MUR	Rupees
			MVR	Rufiyaa
			MWK	Kwachas
			MXN	Pesos
			MYR	Ringgits
			MZM	Meticais
			NAD	Dollars
			NGN	Nairas
			NIO	Gold Cordobas
			NLG	Guilders
			NOK	Krone
			NPR	Nepal Rupees
			NZD	Dollars
			OMR	Rials
			PAB	Balboa
			PEN	Nuevos Soles
			PGK	Kina
			PHP	Pesos
			PKR	Rupees
			PLN	Zlotych
			PTE	Escudos
			PYG	Guarani
			QAR	Rials
			ROL	Lei
			RUR	Rubles
			RWF	Rwanda Francs
			SAR	Riyals
			SBD	Dollars
			SCR	Rupees
			SDD	Dinars
			SEK	Kronor
			SGD	Dollars
			SHP	Pounds
			SIT	Tolars
			SKK	Koruny
			SLL	Leones
			SOS	Shillings
			SPL	Luigini
			SRG	Guilders
			STD	Dobras
			SVC	Colones
			SYP	Pounds
			SZL	Emalangeni
			THB	Baht
			TJR	Rubles
			TMM	Manats
			TND	Dinars
			TOP	Pa'anga
			TRL	Liras
			TTD	Dollars
			TVD	Tuvalu Dollars
			TWD	New Dollars
			TZS	Shillings
			UAH	Hryvnia
			UGX	Shillings
			USD	Dollars
			UYU	Pesos
			UZS	Sums
			VAL	Lire
			VEB	Bolivares
			VND	Dong
			VUV	Vatu
			WST	Tala
			XAF	Francs
			XAG	Ounces
			XAU	Ounces
			XCD	Dollars
			XDR	Special Drawing Rights
			XPD	Ounces
			XPF	Francs
			XPT	Ounces
			YER	Rials
			YUM	New Dinars
			ZAR	Rand
			ZMK	Kwacha
			ZWD	Zimbabwe Dollars

		*/

		return getid3_lib::EmbeddedLookup($currencyid, $begin, __LINE__, __FILE__, 'id3v2-currency-units');
	}


	function LookupCurrencyCountry($currencyid) {

		$begin = __LINE__;

		/** This is not a comment!

			AED	United Arab Emirates
			AFA	Afghanistan
			ALL	Albania
			AMD	Armenia
			ANG	Netherlands Antilles
			AOA	Angola
			ARS	Argentina
			ATS	Austria
			AUD	Australia
			AWG	Aruba
			AZM	Azerbaijan
			BAM	Bosnia and Herzegovina
			BBD	Barbados
			BDT	Bangladesh
			BEF	Belgium
			BGL	Bulgaria
			BHD	Bahrain
			BIF	Burundi
			BMD	Bermuda
			BND	Brunei Darussalam
			BOB	Bolivia
			BRL	Brazil
			BSD	Bahamas
			BTN	Bhutan
			BWP	Botswana
			BYR	Belarus
			BZD	Belize
			CAD	Canada
			CDF	Congo/Kinshasa
			CHF	Switzerland
			CLP	Chile
			CNY	China
			COP	Colombia
			CRC	Costa Rica
			CUP	Cuba
			CVE	Cape Verde
			CYP	Cyprus
			CZK	Czech Republic
			DEM	Germany
			DJF	Djibouti
			DKK	Denmark
			DOP	Dominican Republic
			DZD	Algeria
			EEK	Estonia
			EGP	Egypt
			ERN	Eritrea
			ESP	Spain
			ETB	Ethiopia
			EUR	Euro Member Countries
			FIM	Finland
			FJD	Fiji
			FKP	Falkland Islands (Malvinas)
			FRF	France
			GBP	United Kingdom
			GEL	Georgia
			GGP	Guernsey
			GHC	Ghana
			GIP	Gibraltar
			GMD	Gambia
			GNF	Guinea
			GRD	Greece
			GTQ	Guatemala
			GYD	Guyana
			HKD	Hong Kong
			HNL	Honduras
			HRK	Croatia
			HTG	Haiti
			HUF	Hungary
			IDR	Indonesia
			IEP	Ireland (Eire)
			ILS	Israel
			IMP	Isle of Man
			INR	India
			IQD	Iraq
			IRR	Iran
			ISK	Iceland
			ITL	Italy
			JEP	Jersey
			JMD	Jamaica
			JOD	Jordan
			JPY	Japan
			KES	Kenya
			KGS	Kyrgyzstan
			KHR	Cambodia
			KMF	Comoros
			KPW	Korea
			KWD	Kuwait
			KYD	Cayman Islands
			KZT	Kazakstan
			LAK	Laos
			LBP	Lebanon
			LKR	Sri Lanka
			LRD	Liberia
			LSL	Lesotho
			LTL	Lithuania
			LUF	Luxembourg
			LVL	Latvia
			LYD	Libya
			MAD	Morocco
			MDL	Moldova
			MGF	Madagascar
			MKD	Macedonia
			MMK	Myanmar (Burma)
			MNT	Mongolia
			MOP	Macau
			MRO	Mauritania
			MTL	Malta
			MUR	Mauritius
			MVR	Maldives (Maldive Islands)
			MWK	Malawi
			MXN	Mexico
			MYR	Malaysia
			MZM	Mozambique
			NAD	Namibia
			NGN	Nigeria
			NIO	Nicaragua
			NLG	Netherlands (Holland)
			NOK	Norway
			NPR	Nepal
			NZD	New Zealand
			OMR	Oman
			PAB	Panama
			PEN	Peru
			PGK	Papua New Guinea
			PHP	Philippines
			PKR	Pakistan
			PLN	Poland
			PTE	Portugal
			PYG	Paraguay
			QAR	Qatar
			ROL	Romania
			RUR	Russia
			RWF	Rwanda
			SAR	Saudi Arabia
			SBD	Solomon Islands
			SCR	Seychelles
			SDD	Sudan
			SEK	Sweden
			SGD	Singapore
			SHP	Saint Helena
			SIT	Slovenia
			SKK	Slovakia
			SLL	Sierra Leone
			SOS	Somalia
			SPL	Seborga
			SRG	Suriname
			STD	São Tome and Principe
			SVC	El Salvador
			SYP	Syria
			SZL	Swaziland
			THB	Thailand
			TJR	Tajikistan
			TMM	Turkmenistan
			TND	Tunisia
			TOP	Tonga
			TRL	Turkey
			TTD	Trinidad and Tobago
			TVD	Tuvalu
			TWD	Taiwan
			TZS	Tanzania
			UAH	Ukraine
			UGX	Uganda
			USD	United States of America
			UYU	Uruguay
			UZS	Uzbekistan
			VAL	Vatican City
			VEB	Venezuela
			VND	Viet Nam
			VUV	Vanuatu
			WST	Samoa
			XAF	Communauté Financière Africaine
			XAG	Silver
			XAU	Gold
			XCD	East Caribbean
			XDR	International Monetary Fund
			XPD	Palladium
			XPF	Comptoirs Français du Pacifique
			XPT	Platinum
			YER	Yemen
			YUM	Yugoslavia
			ZAR	South Africa
			ZMK	Zambia
			ZWD	Zimbabwe

		*/

		return getid3_lib::EmbeddedLookup($currencyid, $begin, __LINE__, __FILE__, 'id3v2-currency-country');
	}



	function LanguageLookup($languagecode, $casesensitive=false) {

		if (!$casesensitive) {
			$languagecode = strtolower($languagecode);
		}

		// http://www.id3.org/id3v2.4.0-structure.txt
		// [4.   ID3v2 frame overview]
		// The three byte language field, present in several frames, is used to
		// describe the language of the frame's content, according to ISO-639-2
		// [ISO-639-2]. The language should be represented in lower case. If the
		// language is not known the string "XXX" should be used.


		// ISO 639-2 - http://www.id3.org/iso639-2.html

		$begin = __LINE__;

		/** This is not a comment!

			XXX	unknown
			xxx	unknown
			aar	Afar
			abk	Abkhazian
			ace	Achinese
			ach	Acoli
			ada	Adangme
			afa	Afro-Asiatic (Other)
			afh	Afrihili
			afr	Afrikaans
			aka	Akan
			akk	Akkadian
			alb	Albanian
			ale	Aleut
			alg	Algonquian Languages
			amh	Amharic
			ang	English, Old (ca. 450-1100)
			apa	Apache Languages
			ara	Arabic
			arc	Aramaic
			arm	Armenian
			arn	Araucanian
			arp	Arapaho
			art	Artificial (Other)
			arw	Arawak
			asm	Assamese
			ath	Athapascan Languages
			ava	Avaric
			ave	Avestan
			awa	Awadhi
			aym	Aymara
			aze	Azerbaijani
			bad	Banda
			bai	Bamileke Languages
			bak	Bashkir
			bal	Baluchi
			bam	Bambara
			ban	Balinese
			baq	Basque
			bas	Basa
			bat	Baltic (Other)
			bej	Beja
			bel	Byelorussian
			bem	Bemba
			ben	Bengali
			ber	Berber (Other)
			bho	Bhojpuri
			bih	Bihari
			bik	Bikol
			bin	Bini
			bis	Bislama
			bla	Siksika
			bnt	Bantu (Other)
			bod	Tibetan
			bra	Braj
			bre	Breton
			bua	Buriat
			bug	Buginese
			bul	Bulgarian
			bur	Burmese
			cad	Caddo
			cai	Central American Indian (Other)
			car	Carib
			cat	Catalan
			cau	Caucasian (Other)
			ceb	Cebuano
			cel	Celtic (Other)
			ces	Czech
			cha	Chamorro
			chb	Chibcha
			che	Chechen
			chg	Chagatai
			chi	Chinese
			chm	Mari
			chn	Chinook jargon
			cho	Choctaw
			chr	Cherokee
			chu	Church Slavic
			chv	Chuvash
			chy	Cheyenne
			cop	Coptic
			cor	Cornish
			cos	Corsican
			cpe	Creoles and Pidgins, English-based (Other)
			cpf	Creoles and Pidgins, French-based (Other)
			cpp	Creoles and Pidgins, Portuguese-based (Other)
			cre	Cree
			crp	Creoles and Pidgins (Other)
			cus	Cushitic (Other)
			cym	Welsh
			cze	Czech
			dak	Dakota
			dan	Danish
			del	Delaware
			deu	German
			din	Dinka
			div	Divehi
			doi	Dogri
			dra	Dravidian (Other)
			dua	Duala
			dum	Dutch, Middle (ca. 1050-1350)
			dut	Dutch
			dyu	Dyula
			dzo	Dzongkha
			efi	Efik
			egy	Egyptian (Ancient)
			eka	Ekajuk
			ell	Greek, Modern (1453-)
			elx	Elamite
			eng	English
			enm	English, Middle (ca. 1100-1500)
			epo	Esperanto
			esk	Eskimo (Other)
			esl	Spanish
			est	Estonian
			eus	Basque
			ewe	Ewe
			ewo	Ewondo
			fan	Fang
			fao	Faroese
			fas	Persian
			fat	Fanti
			fij	Fijian
			fin	Finnish
			fiu	Finno-Ugrian (Other)
			fon	Fon
			fra	French
			fre	French
			frm	French, Middle (ca. 1400-1600)
			fro	French, Old (842- ca. 1400)
			fry	Frisian
			ful	Fulah
			gaa	Ga
			gae	Gaelic (Scots)
			gai	Irish
			gay	Gayo
			gdh	Gaelic (Scots)
			gem	Germanic (Other)
			geo	Georgian
			ger	German
			gez	Geez
			gil	Gilbertese
			glg	Gallegan
			gmh	German, Middle High (ca. 1050-1500)
			goh	German, Old High (ca. 750-1050)
			gon	Gondi
			got	Gothic
			grb	Grebo
			grc	Greek, Ancient (to 1453)
			gre	Greek, Modern (1453-)
			grn	Guarani
			guj	Gujarati
			hai	Haida
			hau	Hausa
			haw	Hawaiian
			heb	Hebrew
			her	Herero
			hil	Hiligaynon
			him	Himachali
			hin	Hindi
			hmo	Hiri Motu
			hun	Hungarian
			hup	Hupa
			hye	Armenian
			iba	Iban
			ibo	Igbo
			ice	Icelandic
			ijo	Ijo
			iku	Inuktitut
			ilo	Iloko
			ina	Interlingua (International Auxiliary language Association)
			inc	Indic (Other)
			ind	Indonesian
			ine	Indo-European (Other)
			ine	Interlingue
			ipk	Inupiak
			ira	Iranian (Other)
			iri	Irish
			iro	Iroquoian uages
			isl	Icelandic
			ita	Italian
			jav	Javanese
			jaw	Javanese
			jpn	Japanese
			jpr	Judeo-Persian
			jrb	Judeo-Arabic
			kaa	Kara-Kalpak
			kab	Kabyle
			kac	Kachin
			kal	Greenlandic
			kam	Kamba
			kan	Kannada
			kar	Karen
			kas	Kashmiri
			kat	Georgian
			kau	Kanuri
			kaw	Kawi
			kaz	Kazakh
			kha	Khasi
			khi	Khoisan (Other)
			khm	Khmer
			kho	Khotanese
			kik	Kikuyu
			kin	Kinyarwanda
			kir	Kirghiz
			kok	Konkani
			kom	Komi
			kon	Kongo
			kor	Korean
			kpe	Kpelle
			kro	Kru
			kru	Kurukh
			kua	Kuanyama
			kum	Kumyk
			kur	Kurdish
			kus	Kusaie
			kut	Kutenai
			lad	Ladino
			lah	Lahnda
			lam	Lamba
			lao	Lao
			lat	Latin
			lav	Latvian
			lez	Lezghian
			lin	Lingala
			lit	Lithuanian
			lol	Mongo
			loz	Lozi
			ltz	Letzeburgesch
			lub	Luba-Katanga
			lug	Ganda
			lui	Luiseno
			lun	Lunda
			luo	Luo (Kenya and Tanzania)
			mac	Macedonian
			mad	Madurese
			mag	Magahi
			mah	Marshall
			mai	Maithili
			mak	Macedonian
			mak	Makasar
			mal	Malayalam
			man	Mandingo
			mao	Maori
			map	Austronesian (Other)
			mar	Marathi
			mas	Masai
			max	Manx
			may	Malay
			men	Mende
			mga	Irish, Middle (900 - 1200)
			mic	Micmac
			min	Minangkabau
			mis	Miscellaneous (Other)
			mkh	Mon-Kmer (Other)
			mlg	Malagasy
			mlt	Maltese
			mni	Manipuri
			mno	Manobo Languages
			moh	Mohawk
			mol	Moldavian
			mon	Mongolian
			mos	Mossi
			mri	Maori
			msa	Malay
			mul	Multiple Languages
			mun	Munda Languages
			mus	Creek
			mwr	Marwari
			mya	Burmese
			myn	Mayan Languages
			nah	Aztec
			nai	North American Indian (Other)
			nau	Nauru
			nav	Navajo
			nbl	Ndebele, South
			nde	Ndebele, North
			ndo	Ndongo
			nep	Nepali
			new	Newari
			nic	Niger-Kordofanian (Other)
			niu	Niuean
			nla	Dutch
			nno	Norwegian (Nynorsk)
			non	Norse, Old
			nor	Norwegian
			nso	Sotho, Northern
			nub	Nubian Languages
			nya	Nyanja
			nym	Nyamwezi
			nyn	Nyankole
			nyo	Nyoro
			nzi	Nzima
			oci	Langue d'Oc (post 1500)
			oji	Ojibwa
			ori	Oriya
			orm	Oromo
			osa	Osage
			oss	Ossetic
			ota	Turkish, Ottoman (1500 - 1928)
			oto	Otomian Languages
			paa	Papuan-Australian (Other)
			pag	Pangasinan
			pal	Pahlavi
			pam	Pampanga
			pan	Panjabi
			pap	Papiamento
			pau	Palauan
			peo	Persian, Old (ca 600 - 400 B.C.)
			per	Persian
			phn	Phoenician
			pli	Pali
			pol	Polish
			pon	Ponape
			por	Portuguese
			pra	Prakrit uages
			pro	Provencal, Old (to 1500)
			pus	Pushto
			que	Quechua
			raj	Rajasthani
			rar	Rarotongan
			roa	Romance (Other)
			roh	Rhaeto-Romance
			rom	Romany
			ron	Romanian
			rum	Romanian
			run	Rundi
			rus	Russian
			sad	Sandawe
			sag	Sango
			sah	Yakut
			sai	South American Indian (Other)
			sal	Salishan Languages
			sam	Samaritan Aramaic
			san	Sanskrit
			sco	Scots
			scr	Serbo-Croatian
			sel	Selkup
			sem	Semitic (Other)
			sga	Irish, Old (to 900)
			shn	Shan
			sid	Sidamo
			sin	Singhalese
			sio	Siouan Languages
			sit	Sino-Tibetan (Other)
			sla	Slavic (Other)
			slk	Slovak
			slo	Slovak
			slv	Slovenian
			smi	Sami Languages
			smo	Samoan
			sna	Shona
			snd	Sindhi
			sog	Sogdian
			som	Somali
			son	Songhai
			sot	Sotho, Southern
			spa	Spanish
			sqi	Albanian
			srd	Sardinian
			srr	Serer
			ssa	Nilo-Saharan (Other)
			ssw	Siswant
			ssw	Swazi
			suk	Sukuma
			sun	Sudanese
			sus	Susu
			sux	Sumerian
			sve	Swedish
			swa	Swahili
			swe	Swedish
			syr	Syriac
			tah	Tahitian
			tam	Tamil
			tat	Tatar
			tel	Telugu
			tem	Timne
			ter	Tereno
			tgk	Tajik
			tgl	Tagalog
			tha	Thai
			tib	Tibetan
			tig	Tigre
			tir	Tigrinya
			tiv	Tivi
			tli	Tlingit
			tmh	Tamashek
			tog	Tonga (Nyasa)
			ton	Tonga (Tonga Islands)
			tru	Truk
			tsi	Tsimshian
			tsn	Tswana
			tso	Tsonga
			tuk	Turkmen
			tum	Tumbuka
			tur	Turkish
			tut	Altaic (Other)
			twi	Twi
			tyv	Tuvinian
			uga	Ugaritic
			uig	Uighur
			ukr	Ukrainian
			umb	Umbundu
			und	Undetermined
			urd	Urdu
			uzb	Uzbek
			vai	Vai
			ven	Venda
			vie	Vietnamese
			vol	Volapük
			vot	Votic
			wak	Wakashan Languages
			wal	Walamo
			war	Waray
			was	Washo
			wel	Welsh
			wen	Sorbian Languages
			wol	Wolof
			xho	Xhosa
			yao	Yao
			yap	Yap
			yid	Yiddish
			yor	Yoruba
			zap	Zapotec
			zen	Zenaga
			zha	Zhuang
			zho	Chinese
			zul	Zulu
			zun	Zuni

		*/

		return getid3_lib::EmbeddedLookup($languagecode, $begin, __LINE__, __FILE__, 'id3v2-languagecode');
	}


	function ETCOEventLookup($index) {
        if (($index >= 0x17) && ($index <= 0xDF)) {
		    return 'reserved for future use';
	    }
		if (($index >= 0xE0) && ($index <= 0xEF)) {
			return 'not predefined synch 0-F';
		}
		if (($index >= 0xF0) && ($index <= 0xFC)) {
			return 'reserved for future use';
		}

		static $EventLookup = array(
			0x00 => 'padding (has no meaning)',
			0x01 => 'end of initial silence',
			0x02 => 'intro start',
			0x03 => 'main part start',
			0x04 => 'outro start',
			0x05 => 'outro end',
			0x06 => 'verse start',
			0x07 => 'refrain start',
			0x08 => 'interlude start',
			0x09 => 'theme start',
			0x0A => 'variation start',
			0x0B => 'key change',
			0x0C => 'time change',
			0x0D => 'momentary unwanted noise (Snap, Crackle & Pop)',
			0x0E => 'sustained noise',
			0x0F => 'sustained noise end',
			0x10 => 'intro end',
			0x11 => 'main part end',
			0x12 => 'verse end',
			0x13 => 'refrain end',
			0x14 => 'theme end',
			0x15 => 'profanity',
			0x16 => 'profanity end',
			0xFD => 'audio end (start of silence)',
			0xFE => 'audio file ends',
			0xFF => 'one more byte of events follows'
		);

		return (isset($EventLookup[$index]) ? $EventLookup[$index] : '');
	}

	function SYTLContentTypeLookup($index) {
		static $SYTLContentTypeLookup = array(
			0x00 => 'other',
			0x01 => 'lyrics',
			0x02 => 'text transcription',
			0x03 => 'movement/part name', // (e.g. 'Adagio')
			0x04 => 'events',             // (e.g. 'Don Quijote enters the stage')
			0x05 => 'chord',              // (e.g. 'Bb F Fsus')
			0x06 => 'trivia/\'pop up\' information',
			0x07 => 'URLs to webpages',
			0x08 => 'URLs to images'
		);

		return (isset($SYTLContentTypeLookup[$index]) ? $SYTLContentTypeLookup[$index] : '');
	}

	function APICPictureTypeLookup($index, $returnarray=false) {
		static $APICPictureTypeLookup = array(
			0x00 => 'Other',
			0x01 => '32x32 pixels \'file icon\' (PNG only)',
			0x02 => 'Other file icon',
			0x03 => 'Cover (front)',
			0x04 => 'Cover (back)',
			0x05 => 'Leaflet page',
			0x06 => 'Media (e.g. label side of CD)',
			0x07 => 'Lead artist/lead performer/soloist',
			0x08 => 'Artist/performer',
			0x09 => 'Conductor',
			0x0A => 'Band/Orchestra',
			0x0B => 'Composer',
			0x0C => 'Lyricist/text writer',
			0x0D => 'Recording Location',
			0x0E => 'During recording',
			0x0F => 'During performance',
			0x10 => 'Movie/video screen capture',
			0x11 => 'A bright coloured fish',
			0x12 => 'Illustration',
			0x13 => 'Band/artist logotype',
			0x14 => 'Publisher/Studio logotype'
		);
		if ($returnarray) {
			return $APICPictureTypeLookup;
		}
		return (isset($APICPictureTypeLookup[$index]) ? $APICPictureTypeLookup[$index] : '');
	}

	function COMRReceivedAsLookup($index) {
		static $COMRReceivedAsLookup = array(
			0x00 => 'Other',
			0x01 => 'Standard CD album with other songs',
			0x02 => 'Compressed audio on CD',
			0x03 => 'File over the Internet',
			0x04 => 'Stream over the Internet',
			0x05 => 'As note sheets',
			0x06 => 'As note sheets in a book with other sheets',
			0x07 => 'Music on other media',
			0x08 => 'Non-musical merchandise'
		);

		return (isset($COMRReceivedAsLookup[$index]) ? $COMRReceivedAsLookup[$index] : '');
	}

	function RVA2ChannelTypeLookup($index) {
		static $RVA2ChannelTypeLookup = array(
			0x00 => 'Other',
			0x01 => 'Master volume',
			0x02 => 'Front right',
			0x03 => 'Front left',
			0x04 => 'Back right',
			0x05 => 'Back left',
			0x06 => 'Front centre',
			0x07 => 'Back centre',
			0x08 => 'Subwoofer'
		);

		return (isset($RVA2ChannelTypeLookup[$index]) ? $RVA2ChannelTypeLookup[$index] : '');
	}

	function FrameNameLongLookup($framename) {

		$begin = __LINE__;

		/** This is not a comment!

			AENC	Audio encryption
			APIC	Attached picture
			ASPI	Audio seek point index
			BUF	Recommended buffer size
			CNT	Play counter
			COM	Comments
			COMM	Comments
			COMR	Commercial frame
			CRA	Audio encryption
			CRM	Encrypted meta frame
			ENCR	Encryption method registration
			EQU	Equalisation
			EQU2	Equalisation (2)
			EQUA	Equalisation
			ETC	Event timing codes
			ETCO	Event timing codes
			GEO	General encapsulated object
			GEOB	General encapsulated object
			GRID	Group identification registration
			IPL	Involved people list
			IPLS	Involved people list
			LINK	Linked information
			LNK	Linked information
			MCDI	Music CD identifier
			MCI	Music CD Identifier
			MLL	MPEG location lookup table
			MLLT	MPEG location lookup table
			OWNE	Ownership frame
			PCNT	Play counter
			PIC	Attached picture
			POP	Popularimeter
			POPM	Popularimeter
			POSS	Position synchronisation frame
			PRIV	Private frame
			RBUF	Recommended buffer size
			REV	Reverb
			RVA	Relative volume adjustment
			RVA2	Relative volume adjustment (2)
			RVAD	Relative volume adjustment
			RVRB	Reverb
			SEEK	Seek frame
			SIGN	Signature frame
			SLT	Synchronised lyric/text
			STC	Synced tempo codes
			SYLT	Synchronised lyric/text
			SYTC	Synchronised tempo codes
			TAL	Album/Movie/Show title
			TALB	Album/Movie/Show title
			TBP	BPM (Beats Per Minute)
			TBPM	BPM (beats per minute)
			TCM	Composer
			TCO	Content type
			TCOM	Composer
			TCON	Content type
			TCOP	Copyright message
			TCR	Copyright message
			TDA	Date
			TDAT	Date
			TDEN	Encoding time
			TDLY	Playlist delay
			TDOR	Original release time
			TDRC	Recording time
			TDRL	Release time
			TDTG	Tagging time
			TDY	Playlist delay
			TEN	Encoded by
			TENC	Encoded by
			TEXT	Lyricist/Text writer
			TFLT	File type
			TFT	File type
			TIM	Time
			TIME	Time
			TIPL	Involved people list
			TIT1	Content group description
			TIT2	Title/songname/content description
			TIT3	Subtitle/Description refinement
			TKE	Initial key
			TKEY	Initial key
			TLA	Language(s)
			TLAN	Language(s)
			TLE	Length
			TLEN	Length
			TMCL	Musician credits list
			TMED	Media type
			TMOO	Mood
			TMT	Media type
			TOA	Original artist(s)/performer(s)
			TOAL	Original album/movie/show title
			TOF	Original filename
			TOFN	Original filename
			TOL	Original Lyricist(s)/text writer(s)
			TOLY	Original lyricist(s)/text writer(s)
			TOPE	Original artist(s)/performer(s)
			TOR	Original release year
			TORY	Original release year
			TOT	Original album/Movie/Show title
			TOWN	File owner/licensee
			TP1	Lead artist(s)/Lead performer(s)/Soloist(s)/Performing group
			TP2	Band/Orchestra/Accompaniment
			TP3	Conductor/Performer refinement
			TP4	Interpreted, remixed, or otherwise modified by
			TPA	Part of a set
			TPB	Publisher
			TPE1	Lead performer(s)/Soloist(s)
			TPE2	Band/orchestra/accompaniment
			TPE3	Conductor/performer refinement
			TPE4	Interpreted, remixed, or otherwise modified by
			TPOS	Part of a set
			TPRO	Produced notice
			TPUB	Publisher
			TRC	ISRC (International Standard Recording Code)
			TRCK	Track number/Position in set
			TRD	Recording dates
			TRDA	Recording dates
			TRK	Track number/Position in set
			TRSN	Internet radio station name
			TRSO	Internet radio station owner
			TSI	Size
			TSIZ	Size
			TSOA	Album sort order
			TSOP	Performer sort order
			TSOT	Title sort order
			TSRC	ISRC (international standard recording code)
			TSS	Software/hardware and settings used for encoding
			TSSE	Software/Hardware and settings used for encoding
			TSST	Set subtitle
			TT1	Content group description
			TT2	Title/Songname/Content description
			TT3	Subtitle/Description refinement
			TXT	Lyricist/text writer
			TXX	User defined text information frame
			TXXX	User defined text information frame
			TYE	Year
			TYER	Year
			UFI	Unique file identifier
			UFID	Unique file identifier
			ULT	Unsychronised lyric/text transcription
			USER	Terms of use
			USLT	Unsynchronised lyric/text transcription
			WAF	Official audio file webpage
			WAR	Official artist/performer webpage
			WAS	Official audio source webpage
			WCM	Commercial information
			WCOM	Commercial information
			WCOP	Copyright/Legal information
			WCP	Copyright/Legal information
			WOAF	Official audio file webpage
			WOAR	Official artist/performer webpage
			WOAS	Official audio source webpage
			WORS	Official Internet radio station homepage
			WPAY	Payment
			WPB	Publishers official webpage
			WPUB	Publishers official webpage
			WXX	User defined URL link frame
			WXXX	User defined URL link frame
			TFEA	Featured Artist
			TSTU	Recording Studio
			rgad	Replay Gain Adjustment

		*/

		return getid3_lib::EmbeddedLookup($framename, $begin, __LINE__, __FILE__, 'id3v2-framename_long');

		// Last three:
		// from Helium2 [www.helium2.com]
		// from http://privatewww.essex.ac.uk/~djmrob/replaygain/file_format_id3v2.html
	}


	function FrameNameShortLookup($framename) {

		$begin = __LINE__;

		/** This is not a comment!

			COM	comment
			COMM	comment
			TAL	album
			TALB	album
			TBP	bpm
			TBPM	bpm
			TCM	composer
			TCO	genre
			TCOM	composer
			TCON	genre
			TCOP	copyright
			TCR	copyright
			TEN	encoded_by
			TENC	encoded_by
			TEXT	lyricist
			TIT1	description
			TIT2	title
			TIT3	subtitle
			TLA	language
			TLAN	language
			TLE	length
			TLEN	length
			TMOO	mood
			TOA	original_artist
			TOAL	original_album
			TOF	original_filename
			TOFN	original_filename
			TOL	original_lyricist
			TOLY	original_lyricist
			TOPE	original_artist
			TOT	original_album
			TP1	artist
			TP2	band
			TP3	conductor
			TP4	remixer
			TPB	publisher
			TPE1	artist
			TPE2	band
			TPE3	conductor
			TPE4	remixer
			TPUB	publisher
			TRC	isrc
			TRCK	track
			TRK	track
			TSI	size
			TSIZ	size
			TSRC	isrc
			TSS	encoder_settings
			TSSE	encoder_settings
			TSST	subtitle
			TT1	description
			TT2	title
			TT3	subtitle
			TXT	lyricist
			TXX	text
			TXXX	text
			TYE	year
			TYER	year
			UFI	unique_file_identifier
			UFID	unique_file_identifier
			ULT	unsychronised_lyric
			USER	terms_of_use
			USLT	unsynchronised lyric
			WAF	url_file
			WAR	url_artist
			WAS	url_source
			WCOP	copyright
			WCP	copyright
			WOAF	url_file
			WOAR	url_artist
			WOAS	url_source
			WORS	url_station
			WPB	url_publisher
			WPUB	url_publisher
			WXX	url_user
			WXXX	url_user
			TFEA	featured_artist
			TSTU	studio

		*/

		return getid3_lib::EmbeddedLookup($framename, $begin, __LINE__, __FILE__, 'id3v2-framename_short');
	}

	function TextEncodingTerminatorLookup($encoding) {
		// http://www.id3.org/id3v2.4.0-structure.txt
		// Frames that allow different types of text encoding contains a text encoding description byte. Possible encodings:
		// $00  ISO-8859-1. Terminated with $00.
		// $01  UTF-16 encoded Unicode with BOM. All strings in the same frame SHALL have the same byteorder. Terminated with $00 00.
		// $02  UTF-16BE encoded Unicode without BOM. Terminated with $00 00.
		// $03  UTF-8 encoded Unicode. Terminated with $00.

		static $TextEncodingTerminatorLookup = array(0=>"\x00", 1=>"\x00\x00", 2=>"\x00\x00", 3=>"\x00", 255=>"\x00\x00");

		return @$TextEncodingTerminatorLookup[$encoding];
	}

	function TextEncodingNameLookup($encoding) {
		// http://www.id3.org/id3v2.4.0-structure.txt
		static $TextEncodingNameLookup = array(0=>'ISO-8859-1', 1=>'UTF-16', 2=>'UTF-16BE', 3=>'UTF-8', 255=>'UTF-16BE');
		return (isset($TextEncodingNameLookup[$encoding]) ? $TextEncodingNameLookup[$encoding] : 'ISO-8859-1');
	}

	function IsValidID3v2FrameName($framename, $id3v2majorversion) {
		switch ($id3v2majorversion) {
			case 2:
				return ereg('[A-Z][A-Z0-9]{2}', $framename);
				break;

			case 3:
			case 4:
				return ereg('[A-Z][A-Z0-9]{3}', $framename);
				break;
		}
		return false;
	}

	function IsANumber($numberstring, $allowdecimal=false, $allownegative=false) {
		for ($i = 0; $i < strlen($numberstring); $i++) {
			if ((chr($numberstring{$i}) < chr('0')) || (chr($numberstring{$i}) > chr('9'))) {
				if (($numberstring{$i} == '.') && $allowdecimal) {
					// allowed
				} elseif (($numberstring{$i} == '-') && $allownegative && ($i == 0)) {
					// allowed
				} else {
					return false;
				}
			}
		}
		return true;
	}

	function IsValidDateStampString($datestamp) {
		if (strlen($datestamp) != 8) {
			return false;
		}
		if (!$this->IsANumber($datestamp, false)) {
			return false;
		}
		$year  = substr($datestamp, 0, 4);
		$month = substr($datestamp, 4, 2);
		$day   = substr($datestamp, 6, 2);
		if (($year == 0) || ($month == 0) || ($day == 0)) {
			return false;
		}
		if ($month > 12) {
			return false;
		}
		if ($day > 31) {
			return false;
		}
		if (($day > 30) && (($month == 4) || ($month == 6) || ($month == 9) || ($month == 11))) {
			return false;
		}
		if (($day > 29) && ($month == 2)) {
			return false;
		}
		return true;
	}

	function ID3v2HeaderLength($majorversion) {
		return (($majorversion == 2) ? 6 : 10);
	}

}

?>