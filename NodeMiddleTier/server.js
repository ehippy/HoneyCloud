var url = require('url');

var AWS = require('aws-sdk'); 
AWS.config.loadFromPath('./awscreds.json');

console.log("HoneyCloud Server Booting...");

var http = require("http");

http.createServer(function(request, response) {
	
  response.writeHead(200, {"Content-Type": "text/plain"});
  response.write("Hello World");
  response.end();
  
  
  var url_parts = url.parse(request.url, true);
  var query = url_parts.query;
  
  console.log(query);
  
  if (query.reading_timestamp > 0) {
	  writeToLogTable(parseInt(query.reading_timestamp), parseFloat(query.reading_temperature));
  } else {
	  console.log("Request didn't have a reading_timestamp");
  }
  
}).listen(process.env.PORT || 8888);


console.log("HoneyCloud Server Listening...");


function writeToLogTable(readingTimeStamp, readingTemperature) {

	var table = new AWS.DynamoDB({params: {TableName: 'hive_readings'}});
	// Write the item to the table
	var itemParams = {
		Item: {
			hive_name: {S: "custer"},
			reading_timestamp: {N: String(readingTimeStamp)},
			inside_temp: {N: String(readingTemperature)}
		}
	};
	
	table.putItem(itemParams, function(err) {
		if (err!=null) {
			console.log(err);
		} else {
			console.log('wrote to table');
		}
	});
	
}