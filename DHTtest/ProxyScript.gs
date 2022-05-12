// Fill Proxy sheet ID here
var ProxySS = SpreadsheetApp.openById(''); 
var ProxySheet = ProxySS.getSheetByName('Mapping');

var hostGoogle = "https://script.google.com";

function testSearch(){
  var result = searchScriptDeployID("DHT Data");
  console.log('resutl: ' + result);
}

function searchScriptDeployID(sheet_script) {
  var mappings = ProxySheet.getRange(2, 1, ProxySheet.getLastRow(), ProxySheet.getLastColumn()).getValues();
  for(var i = 0; i < mappings.length; i++){
    if(mappings[i][0] == sheet_script){
      return mappings[i][1];
    }
  }
  return -1;
}

function forwardPostRequestToTableScript(url, e) {
  var options = {
    'method': 'post',
    'contentType': 'application/json',
    'payload': e.postData.contents
  };
  return ContentService.createTextOutput(UrlFetchApp.fetch(url, options)).setMimeType(ContentService.MimeType.TEXT);
}

function doPost(e) {
  var parsedData;

  try {
    parsedData = JSON.parse(e.postData.contents);
  }
  catch (f) {
    return ContentService.createTextOutput("Error in parsing request body: " + f.message);
  }

  if (parsedData !== undefined) {
    var sheetScriptID = searchScriptDeployID(parsedData.sheet_script);
    if(sheetScriptID < 0){
      return ContentService.createTextOutput("Proxy Error: Can't find requested script: " + parsedData.sheet_script);
    }

    var url = hostGoogle + "/macros/s/" + sheetScriptID + "/exec";
    return forwardPostRequestToTableScript(url, e);

  } else {
    return ContentService.createTextOutput("Error! Request body empty or in incorrect format.");
  }
}
