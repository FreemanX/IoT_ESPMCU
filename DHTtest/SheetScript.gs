// Enter Spreadsheet ID here
var SS = SpreadsheetApp.openById('');
var str = "";
var abnormal_temp_low = 10;
var abnormal_temp_high = 26;
var abnormal_humi_low = 10;
var abnormal_humi_high = 60;

function SendinganEmailMessage(email_address, email_subject, email_body) {
  MailApp.sendEmail(email_address, email_subject, email_body);
}

function createChart(sheet) {
  var artistsChart = sheet.newChart().asLineChart()
    .addRange(sheet.getRange("D2:F146")) // 10 min a update, 144 data a day
    .setTitle('24小時總覽') // 24 hour overview
    .setXAxisTitle('時間') // time
    .setYAxisTitle('數值') // value
    .setPosition(4, 7, 0, 0)
    .build();

  sheet.insertChart(artistsChart);
}

function doPost(e) {

  var parsedData;
  var result = {};

  try {
    parsedData = JSON.parse(e.postData.contents);
  }
  catch (f) {
    return ContentService.createTextOutput("Error in parsing request body: " + f.message);
  }

  if (parsedData !== undefined) {

    // sheet name to publish data to is specified in Arduino code
    var sheet = SS.getSheetByName(parsedData.sheet_name);
    if (sheet == null) {
      sheet = SS.insertSheet();
      sheet.setName(parsedData.sheet_name);
      sheet.getRange('A1').setValue("日期"); // date
      sheet.getRange('B1').setValue("時間"); // time
      sheet.getRange('C1').setValue("ID");
      sheet.getRange('D1').setValue("溫度"); // temperature
      sheet.getRange('E1').setValue("濕度"); // humidity
      sheet.getRange('F1').setValue("日期時間"); // datetime
      sheet.getRange('G1').setValue("日期（異常）"); // date (abnormal)
      sheet.getRange('H1').setValue("時間（異常）"); // time (abnormal)
      sheet.getRange('I1').setValue("ID（異常）");
      sheet.getRange('J1').setValue("溫度（異常）"); // temperature (abnormal)
      sheet.getRange('K1').setValue("濕度（異常）"); // humidity (abnormal)
    }
    // creates an array of the values to publish 
    var dataArr = parsedData.values.split(",");
    var timeZone = SpreadsheetApp.getActive().getSpreadsheetTimeZone();
    var date_now = Utilities.formatDate(new Date(), timeZone, "yyyy/MM/dd"); // gets the current date
    var time_now = Utilities.formatDate(new Date(), timeZone, "HH:mm:ss"); // gets the current time
    var date_time = date_now + time_now

    var mac_add = dataArr[0]; // identifier
    var temp = dataArr[1]; // temp
    var humid = dataArr[2]; // humid

    // read and execute command from the "payload_base" string specified in Arduino code
    switch (parsedData.command) {

      case "insert_row":

        // sheet.insertRows(2); // insert full row directly below header text

        var range = sheet.getRange("A2:F2");              // use this to insert cells just above the existing data instead of inserting an entire row
        range.insertCells(SpreadsheetApp.Dimension.ROWS); // use this to insert cells just above the existing data instead of inserting an entire row

        sheet.getRange('A2').setValue(date_now); // publish current date to cell A2
        sheet.getRange('B2').setValue(time_now); // publish current time to cell B2
        sheet.getRange('C2').setValue(mac_add);   // publish mac_add from Arduino code to cell C2
        sheet.getRange('D2').setValue(temp);   // publish temp from Arduino code to cell D2
        sheet.getRange('E2').setValue(humid);   // publish humid from Arduino code to cell E2
        sheet.getRange('F2').setValue(date_time);

        if (temp > abnormal_temp_high || temp < abnormal_temp_low || humid > abnormal_humi_high || humid < abnormal_humi_low) {
          range = sheet.getRange("G2:K2");
          range.insertCells(SpreadsheetApp.Dimension.ROWS);
          sheet.getRange('G2').setValue(date_now); // publish current date to cell A2
          sheet.getRange('H2').setValue(time_now); // publish current time to cell B2
          sheet.getRange('I2').setValue(mac_add);   // publish mac_add from Arduino code to cell C2
          sheet.getRange('J2').setValue(temp);   // publish temp from Arduino code to cell D2
          sheet.getRange('K2').setValue(humid);   // publish humid from Arduino code to cell E2
        }

        str = "Success"; // string to return back to Arduino serial console
        SpreadsheetApp.flush();
        break;

      case "append_row":

        var publish_array = new Array(); // create a new array

        publish_array[0] = date_now; // add current date to position 0 in publish_array
        publish_array[1] = time_now; // add current time to position 1 in publish_array
        publish_array[2] = mac_add;   // add mac_add from Arduino code to position 2 in publish_array
        publish_array[3] = temp;   // add temp from Arduino code to position 3 in publish_array
        publish_array[4] = humid;   // add humid from Arduino code to position 4 in publish_array

        sheet.appendRow(publish_array); // publish data in publish_array after the last row of data in the sheet

        str = "Success"; // string to return back to Arduino serial console
        SpreadsheetApp.flush();
        break;
    }


    return ContentService.createTextOutput(str);
  } // endif (parsedData !== undefined)

  else {
    return ContentService.createTextOutput("Error! Request body empty or in incorrect format.");
  }
}


