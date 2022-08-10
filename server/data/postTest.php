
<?php
    $putdata = fopen("php://input", "r");
    $length = 100;
    while ($data = fread($putdata, 1) && ($length >= 0)) {
        $length--;
    }
    if ($length >= 0) {
        http_response_code(201);
    } else {
        http_response_code(413);
    }
    fclose($putdata);
?>
