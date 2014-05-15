HoneyCloud
==========

A node.js / Arduino / Dynamo mashup to live monitor a beehive


==========

Consists of three major components
 - *Arduino* code to take temperature readings and send them to Node.js via HTTP over wifi
 - *Node.js* code to received HTTP GET requests from the Arduino, and save them to DynamoDB
 - *Website* that draws the Dynamo data on a page