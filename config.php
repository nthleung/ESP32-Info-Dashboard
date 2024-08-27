<?php

    define("APIKEY", "123a456b789c123d456e789");

    header('Content-Type: application/json');

    if($_GET["apiKey"] != APIKEY){
        http_response_code(401);
        echo json_encode(["error" => "Unauthorized"]);
        exit;
    }
    

    echo json_encode(array(
        "weather" => array(
            "location" => array(
                "lat" => "40.6813",
                "lon" => "-73.9762"
            ),
            "showFahrenheit" => true
        ),
        "countdown" => array(
            "enable" => true,
            "events" => array(
                array(
                    "eventName" => "✈Hong Kong",
                    "targetDateTs" => mktime(0, 0, 0, 8, 29, 2025)    //mktime(h, m, s, m, d, y)
                )
            )
        ),
        "barclays" => array(
            "enable" => true,
        ),
        "customMessages" => array(
            "enable" => false,
            "messages" => array (
                "my custom line1",
                "my custom line2"
            )
        ),
    ));

?>