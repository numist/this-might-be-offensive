<?php 

mustLogIn(); 

if($_SESSION['status'] !== "admin") header("Location: ./");

phpinfo(); 

?>
