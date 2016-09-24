<?php

$str = urldecode($_SERVER['QUERY_STRING']);

if ($str != "") {
	$fh = fopen("/tmp/web.state", "w");
	fwrite($fh, $str);
	fclose($fh);
	
	$a = explode(' ', $str);
	$cmd = "apa102";
	foreach ($a as $e) {
		$cmd .= " " . escapeshellarg($e);
	}
	
	system($cmd);
	

	
	
	

} else {
	@readfile("/tmp/web.state");

}




?>
