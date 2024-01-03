#!/usr/bin/env php
<?php
// Ensure that the content type header is set to HTML
header('Content-Type: text/html');

// Parse the QUERY_STRING environment variable
parse_str(getenv('QUERY_STRING'), $params);

// Retrieve data from the query parameters
$name = isset($params['name']) ? $params['name'] : 'Unknown';
$age = isset($params['age']) ? $params['age'] : 'Unknown';

// Generate the HTML content
echo "<html><head><title>CGI Script</title></head>\n";
echo "<body>\n";
echo "<h1>CGI Script Response</h1>\n";
echo "<p>Name: " . htmlspecialchars($name) . "</p>\n";
echo "<p>Age: " . htmlspecialchars($age) . "</p>\n";
echo "</body></html>";
?>
