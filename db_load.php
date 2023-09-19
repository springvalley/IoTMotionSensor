<?php 

//Refresh denne siden hvert 5. sekund.
header("refresh: 5");

//Variabler for å koble til databasen som sensordataene blir sendt til. 
//Denne informasjonen får du når du først lager en server i Xampp og en database.
$hostname = "localhost";
$username = "root";
$password = "";
$db_name = "is311";


//Lag en tilkobling til databasen
$conn = mysqli_connect($hostname, $username, $password, $db_name);


//Hvis vi ikke kan koble til databasen så stopper vi scriptet. 
if(!$conn){
    die("Connection failed:" . mysqli_connect_error());
}

//Hent data fra databasen
$sql = "SELECT * FROM msdata";
$res = mysqli_query($conn, $sql);

$readings = mysqli_fetch_all($res, MYSQLI_ASSOC);

//Foreachloop for å skrive ut hver linje (hver sensormåling) vi får fra databasen. 
foreach($readings as $reading){?>
<p><?php echo "Motion started at: ". htmlspecialchars($reading["motionStart"] . ". Motion ended at: ". htmlspecialchars($reading["motionStop"]. ". The total duration of the motion was: ". htmlspecialchars($reading["motionDuration"])))?></p>
<?php }?>