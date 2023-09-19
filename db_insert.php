<?php 

//Variabler for å koble til databasen som sensordataene blir sendt til. 
//Denne informasjonen får du når du først lager en server i Xampp og en database.

$hostname = "localhost";
$username = "root";
$password = "";
$db_name = "is311";


//Lag en tilkobling til databasen
$conn = mysqli_connect($hostname, $username, $password, $db_name);

//Hvis vi ikke kan koble til databasen stopper vi scriptet.
if(!$conn){
    die("Connection failed:" . mysqli_connect_error());
}

//Hvis ikke requesten vi får til denne siden kommer fra en post request (ardunioen basically)
//Så sender vi de til siden som viser dataen og stopper scriptet. 
if(!isset($_POST["time"])){
    header("location: db_load.php");
    die();
}

//Vi får kun tilsendt hvor lenge bevegelsen har vart fra arduinoen, vi kan derimot regne oss bakover
//med denne tiden fordi vi vet hvor mange sekunder vi har hatt bevegelse. 
$motionDuration = $_POST["time"];
$start = date("Y-m-d H:i:s", time() - $motionDuration) ;
$end = date("Y-m-d H:i:s", time());

//SQL insert for å legge inn dataen i databasen. 
$sql = "INSERT INTO msdata(motionStart, motionStop, motionDuration) VALUES ('".$start."', '".$end."', '".$motionDuration."')";
$res = mysqli_query($conn, $sql);

//Debug hvis vi får en feil. 
echo mysqli_error($conn);
?>