<!DOCTYPE html>
<head>
<html lang="en">
<meta charset="UTF-8">
<title>OCR Results</title>

<link rel="stylesheet" type="text/css" href="style/style1.css">

</head>

<body>
<?php
      exec("/usr/bin/sudo /usr/lib/python /home/pi/ocr/ocr.py");

      Echo "<h1>Display page for OCR results </h1>";

      Echo "<img id='original' src='data/image.png' alt='original image'>";
      Echo "<img id='fixed' src='data/imagefix.png' alt='modified image for ocr'>";

      Echo "<p> Here is what tesseract OCR sees:";
      $str = file_get_contents("data/final.txt");
      Echo "<p><strong>$str</strong></p>";
?>

</body>
</html>
