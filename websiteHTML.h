static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, minimum-scale=1.0, user-scalable=no">
  <meta name="apple-mobile-web-app-capable" content="yes">
  <title>Oscilloscope</title>
  <script>
    var websock;
    var inputString = "";
    var wsMessageArray = "";
    var i2cDeviceArray = [];
    var i2cRegisterArray = [];
    var lastI2CDeviceAddress = null;
    var lastI2CControlRegister = null;
    var channelSelect1 = "OFF";
    var channelSelect2 = "OFF";
    var timeScaleSelect = "2";
    var toggleMenuFlag = true;
    var landscapeOrientationFlag = false;
    var toggleSettingsElementFlag = false;
    var currentScreenElement = "SCOPE";
    var terminalEchoFlag = false;
    var peakDetectionFlag = false;
    var pauseScopeFlag = false;
    var dataChannelOnFlag1 = true;
    var dataChannelOnFlag2 = false;
    var dataLogFlag = false;
    var dataOSDFlag= false;
    var terminalConnectFlag = true;
    //Plot Variables
    var plotCanvasWidth = 1024;
    var plotCanvasHeight = 800; // just defaults...
    var xPlotCurrentPosition = 0.5;
    var xPlotCurrentPosition2 = 0.5;
    var yPlotCurrentPosition = 0;
    var yPlotCurrentPosition2 = 0;
    var xPlotOldPosition = 0;
    var xPlotOldPosition2 = 0;
    var yPlotOldPosition = 0;
    var yPlotOldPosition2 = 0;
     var yPlotScaleFactor = 10;   // DAG nb setting a different start point
     var xPlotScaleFactor = 1;
     var xPlotTotalTimeMax =2;
     //var xPlotWS_Timer = 2000;  // 
     var sampleuSTimer =5000;  // us for samples
     var xPlotSamplesPerSecond = 200; // 
     var currentmst=10;
     var currentuSTimer  = 5000;
    var xPlotPositionStep = 10;  //DAG is used as delta t in plot in scope 
    
    var xPlotTotalTime = 10; 
    var yPlotMax = 64;           // DAG Scope display is basically set up for 2048 INPUT = 64 v
    var channelIncomingYPlotPosition1 = 0;
    var channelIncomingYPlotPosition2 = 0;
    var peakDetectInputValue = 0;
    var peakDetectFirstReadFlag = false;
    var peakDetectMinData;
    var peakDetectMaxData;
    var peakDetectMinMaxAverage;
    var peakDetectWindowScalar = 1;
    var peakDetectTimeOffset = 1;
    var peakDetectOnPeak = false;
    var peakDetectLastPeak = false;
    var peakDetectDataWindowPos;
    var peakDetectDataWindowNeg;
    var peakDetectPeakXValue = 0;
    var peakDetectPeakYValue = 0;
    var peakDetectDataWindowPosYValue = 0;
    var peakDetectDataWindowNegYValue = 0;
    var data2send="";
    var FirstAfterCLS=0;
    var PlotEdgeReached =false;
    var MissingSteps=0;
    var PlotStraightLine=false;
    
    function start()
    
    {
      //websock = new WebSocket('ws://192.168.4.1:81/');
      websock = new WebSocket('ws://' + window.location.hostname + ':81/');
      websock.onopen = function(evt)
      {
        console.log('websock open');
        websock.send("SCOPE CHANNEL 1 INT ADC"); 
        xPlotTotalTimeMax=2;   // 2 sec screen  
      //websock.send("SCOPE CHANNEL 1 SCALES");
        websock.send("SCOPE CHANNEL 2 DIG"); 
      //UpdateMST(xPlotTotalTimeMax,sampleuSTimer); // will update WS_Timer / sampleuSTimerto defaults
        UpdateMST(10,5000); // Set websocks update and adc sample timers 
        //websock.send("SCOPE WS_Timer 200");     // 
        //websock.send("SCOPE Sample_uS 5000");  // dag note max rate for two scales to be read alternately
       websock.send("Function Start completed "); 
        websock.send("Data_accepted "); // sets up flags
        websock.send("Read_CTS ");   // initiates first send.. (not needed for original display.. )
      };
      websock.onclose = function(evt)
      {
        console.log('websock close');
      };
      websock.onerror = function(evt)
      {
        console.log(evt);
      };
      websock.onmessage = function(evt)
      {
        console.log(evt);
        wsMessageArray = evt.data.split(" ");
        if(wsMessageArray.length >= 2)
        {
          if(wsMessageArray[0] === "SCOPE")
          {
            scopeEventHandler();
          }
          if(wsMessageArray[0] === "SERIAL")
          {
            serialEventHandler();
          }
        }
      };
         clearPlot();
    }

    function scopeEventHandler()
    {
       if(wsMessageArray[1] === "ADC")
      {
        if(wsMessageArray[2] === "DATACHANNEL1")
        {
            dataChannelOnFlag1 = true;
      if(wsMessageArray.length > 3)
      { 
        if(PlotEdgeReached)   {  clearPlot();   }// try to only clear plot when new data is ready and waiting ?
       for(var updatePlotCounter = 3; updatePlotCounter <= (wsMessageArray.length-1); updatePlotCounter++)
           { 
             updatePlot(parseInt(wsMessageArray[updatePlotCounter]));
           }
      }
        }
        if(wsMessageArray[2] === "DATACHANNEL2")
        {
            dataChannelOnFlag2 = true;
      if(wsMessageArray.length > 3)
      {
        if(PlotEdgeReached)   {  clearPlot();   }// try to only clear plot when new data is ready and waiting ?
        for(var updatePlotCounter = 3; updatePlotCounter <= (wsMessageArray.length-1); updatePlotCounter++)
        {
          updatePlot(parseInt(wsMessageArray[updatePlotCounter]));
        }
      }
        }
      }
      if(wsMessageArray[1] === "UART")
      {
        if(wsMessageArray[2] === "DATACHANNEL1")
        {
          dataChannelOnFlag1 = true;
      if(wsMessageArray.length>3)
      {
      for(var updatePlotCounter = 3; updatePlotCounter <= (wsMessageArray.length-1); updatePlotCounter++)
      {
        if(!isNaN(parseInt(wsMessageArray[updatePlotCounter])))
        {
          updatePlot(parseInt(wsMessageArray[updatePlotCounter]));
        }
      }
      }
        }
        if(wsMessageArray[2] === "DATACHANNEL2")
        {
      dataChannelOnFlag2 = true;
      if(wsMessageArray.length>3)
      {
      for(var updatePlotCounter = 3; updatePlotCounter <= (wsMessageArray.length-1); updatePlotCounter++)
      {
        if(!isNaN(parseInt(wsMessageArray[updatePlotCounter])))
        {
          updatePlot(parseInt(wsMessageArray[updatePlotCounter]));
        }
      }
      }
        }
      }
      if(wsMessageArray[1] === "OFF")
      {
        if(wsMessageArray[2] === "DATACHANNEL1")
        {
          dataChannelOnFlag1 = false;
          updatePlot(parseInt(wsMessageArray[3]));
        }
        if(wsMessageArray[2] === "DATACHANNEL2")
        {
          dataChannelOnFlag2 = false;
          updatePlot(parseInt(wsMessageArray[3]));
        }
        if((document.getElementById("channelSelectElement1").value === "OFF") && (document.getElementById("channelSelectElement2").value === "OFF"))
        {
          clearPlot();
        }
      }
      if(wsMessageArray[1] === "CLEAR")
      {
        clearPlot();
      }
      if(wsMessageArray[1] === "SETTINGS")
            {
       //websock.send("In scope settings.."); //DAG
        if(wsMessageArray[2] === "YSCALE")
        {
          yPlotScaleFactor = parseInt(wsMessageArray[3]);
          clearPlot();
        }
        if(wsMessageArray[2] === "XSCALE")
        {
          xPlotScaleFactor = parseInt(wsMessageArray[3]);
          clearPlot();
        }
        if(wsMessageArray[2] === "YMAX")
        {
          yPlotMax = parseInt(wsMessageArray[3]);
          clearPlot();
        }
        if(wsMessageArray[2] === "Sample_uS")
        {     
          sampleuSTimer = parseInt(wsMessageArray[3]); 
          xPlotSamplesPerSecond = 1000000 / sampleuSTimer;
          data2send="";
          data2send = "Updated sampleuSTimer to:   ";
          data2send += sampleuSTimer;
          websock.send(data2send);
          clearPlot();
        }
        if(wsMessageArray[2] === "WS_Timer")
        {     
          xPlotTotalTimeMax = parseInt(wsMessageArray[3]); 

          data2send="";
          data2send = "Updated WS_Timer (scope width in seconds) to:   ";
          data2send += xPlotTotalTimeMax;
          websock.send(data2send);
          websock.send(" line 239 WS_Timer set xplottotal time ");
          clearPlot();
        }
      }
    }

    function serialEventHandler()
    {
      if(wsMessageArray[1] === "UART")
      {
        var outputTable = document.getElementById('terminalOutput');
        var terminalOutputWrapperDiv = document.getElementById('terminalOutputWrapper');
        var slicedWsMessageArray = wsMessageArray.slice(2).join(' ');
        outputTable.innerHTML = outputTable.innerHTML + '<tr><td>' + slicedWsMessageArray + '</tr><td>';
        terminalOutputWrapperDiv.scrollTop = terminalOutputWrapperDiv.scrollHeight;
        if(wsMessageArray[2] === "CLEAR")
        {
          terminalClear();
        }
      }
      if(wsMessageArray[1] === "I2C")
      {
        if(wsMessageArray[2] === "DEVICE")
        {
          var i2cOutputDeviceWrapperDiv = document.getElementById('i2cOutputDeviceWrapper');
          i2cOutputDeviceWrapperDiv.innerHTML = i2cOutputDeviceWrapperDiv.innerHTML + '<button id="i2cDevice'+parseInt(wsMessageArray[3], 16)+'" style="-webkit-appearance: none; width: 90%; height: 10vh; margin-bottom:5%; background-color:#E87D75; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica;" onclick="i2cRequestRegisterRead(' + parseInt(wsMessageArray[3], 16) + ')"><b>0x' + wsMessageArray[3].toUpperCase() + '</b></button>';
        }
        if(wsMessageArray[2] === "REGISTER")
        {
          var i2cOutputRegisterWrapperDiv = document.getElementById('i2cOutputRegisterWrapper');
          i2cOutputRegisterWrapperDiv.innerHTML = i2cOutputRegisterWrapperDiv.innerHTML + '<button id="i2cRegister'+parseInt(wsMessageArray[3], 16)+'"  style="-webkit-appearance: none; width: 90%; height: 10vh; margin-bottom:5%; background-color:#E87D75; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica;" onclick="i2cUpdateControlRegister(' + parseInt(wsMessageArray[3], 16) + ')"><b>Reading 0x' + wsMessageArray[3].toUpperCase() + ': 0x' + wsMessageArray[4].toUpperCase() + '</b></button>';
        }
        if(wsMessageArray[2] === "CLEAR")
        {
          i2cClearDeviceField();
          i2cClearRegisterField();
        }
      }
      if(wsMessageArray[1] === "SETTINGS")
      {
        if(wsMessageArray[2] === "YSCALE")
        {
          yPlotScaleFactor = parseInt(wsMessageArray[3]);
          clearPlot();
        }
        if(wsMessageArray[2] === "XSCALE")
        {
          xPlotScaleFactor = parseInt(wsMessageArray[3]);
          clearPlot();
        }
        if(wsMessageArray[2] === "YMAX")
        {
          yPlotMax = parseInt(wsMessageArray[3]);
          clearPlot();
        } 
      }
    }

    
    function UpdateMST(currentmst , currentuSTimer) {  // update scope plot dimensions / sample times tim
      //var changed = "false;
      //xPlotTotalTimeMax=2;   // default 2 sec update 
      sampleuSTimer = 3000; // default 3mS
      if (     (document.getElementById("channelSelectElement1").value==="SCALES") || (document.getElementById("channelSelectElement2").value==="SCALES") || (document.getElementById("channelSelectElement2").value==="SCALESB") || (document.getElementById("channelSelectElement1").value==="SCALES")  ) {
       // xPlotTotalTimeMax = 10;      // 2 sec plot
        sampleuSTimer = 100000;  // 100ms sample
      }
      if (currentuSTimer!=sampleuSTimer) {
      //  changed = true;
        data2send="";
        data2send = "SCOPE Sample_uS ";
        data2send += sampleuSTimer;
        websock.send(data2send);
      }
      if (currentmst!=xPlotTotalTimeMax) {
      //  changed = true;
        data2send="";
        data2send = "SCOPE WS_Timer ";
        data2send += xPlotTotalTimeMax *1000;  // send ms
       websock.send(data2send);
       websock.send(" line 329 WS_Timer set xplottotal time ");
      }
      xPlotSamplesPerSecond = 1000000 / sampleuSTimer;      
    }
    function adjustCanvas()
    {
      var currentClientWidth = (document.documentElement.clientWidth);
      var currentClientHeight = (document.documentElement.clientHeight);
      var plotElementID = document.getElementById("plotElement");
      plotElementID.width = currentClientWidth;
      plotElementID.height = (currentClientHeight * 0.65);
      landscapeOrientationFlag = false;
      plotCanvasWidth = plotElementID.width;
      plotCanvasHeight = plotElementID.height;
      yPlotCurrentPosition = plotCanvasHeight;
      yPlotOldPosition2 = plotCanvasHeight;
      clearPlot();
    }

    function clearPlot()
    {        
      UpdateMST(xPlotTotalTimeMax,sampleuSTimer);
      var plotElementID = document.getElementById("plotElement");
      var pctx = plotElementID.getContext("2d"); 
     // xPlotPositionStep = (((plotCanvasWidth * xPlotTotalTimeMax)/ (xPlotTotalTimeMax * xPlotScaleFactor))/1000); //DAG modified and using WS_Timer   
      //xPlotPositionStep = (((plotCanvasWidth * xPlotTotalTimeMax)/ (xPlotTotalTimeMax * xPlotScaleFactor))/1000); //DAG for scope screen size again    
      xPlotPositionStep = plotCanvasWidth / (xPlotTotalTimeMax * xPlotSamplesPerSecond * xPlotScaleFactor); //original
      pctx.fillStyle = "white";
      pctx.clearRect(0, 0, plotCanvasWidth, plotCanvasHeight);
      pctx.beginPath();
      pctx.fillRect(0, 0, plotCanvasWidth, plotCanvasHeight);
      xPlotOldPosition = 0; 
      xPlotOldPosition2 =0;
      xPlotCurrentPosition = 0;
      xPlotCurrentPosition2 = 0;
      pctx.lineJoin = "round";
      pctx.lineCap = "round";
      pctx.font = (plotCanvasHeight / 25) + "px Helvetica";
      pctx.fillStyle = "#4E4E56";
      pctx.strokeStyle = "#EDEDEE";
      pctx.lineWidth = plotCanvasHeight / 90;
      var xDivisions = 1;
      var Xgrid=10; // added 
    
      for(xDivisions = 1; xDivisions < Xgrid; xDivisions++)
      {
        pctx.beginPath();
        pctx.moveTo((xDivisions * plotCanvasWidth / Xgrid), 0);
        pctx.lineTo((xDivisions * plotCanvasWidth / Xgrid), plotCanvasHeight);
        pctx.closePath();
        pctx.stroke();
        if ((xPlotTotalTimeMax *   xPlotScaleFactor ) <= 5){                // DAG note xplot factor is 1  / the multiplier
        pctx.fillText((xDivisions * xPlotTotalTimeMax * xPlotScaleFactor / Xgrid).toFixed(2) + "s", ((plotCanvasWidth * xDivisions / Xgrid)) - pctx.measureText(
          (xDivisions * xPlotTotalTimeMax * xPlotScaleFactor/ Xgrid).toFixed(2) + "s").width / 2, (plotCanvasHeight - 10));
        }
        else{
            pctx.fillText((xDivisions * xPlotTotalTimeMax * xPlotScaleFactor / Xgrid).toFixed(0) + "s", ((plotCanvasWidth * xDivisions / Xgrid)) - pctx.measureText(
          (xDivisions * xPlotTotalTimeMax * xPlotScaleFactor/ Xgrid).toFixed(0) + "s").width / 2, (plotCanvasHeight - 10));
        }
    
      }

      
      var yDivisions = 1;
      var   VGridLines =6 ; // (DAG this was original number of horizonmtal lines +1 
      var VScale = VGridLines*64/60   ;   // DAG yes this is horrid, I should have modified the code differently.. 
      for(yDivisions = 1; yDivisions < VGridLines; yDivisions++)
      {
        pctx.beginPath();
        pctx.moveTo(0, plotCanvasHeight-(yDivisions* plotCanvasHeight / VScale));
        pctx.lineTo(plotCanvasWidth, plotCanvasHeight-(yDivisions* plotCanvasHeight / VScale));
        pctx.closePath();
        pctx.stroke();
        //DAG revised this function and removed "V" as it can be kG or Volts.. pctx.fillText(( yDivisions / VScale * (yPlotMax) / yPlotScaleFactor).toFixed(1) + " ", 5, (plotCanvasHeight-(yDivisions* plotCanvasHeight / VScale) - 5));}
       if((document.getElementById("channelSelectElement1").value==="SCALES")||(document.getElementById("channelSelectElement2").value==="SCALES")||(document.getElementById("channelSelectElement2").value==="SCALESB")){
        pctx.fillText((( yDivisions / VScale * (yPlotMax) / yPlotScaleFactor)-1).toFixed(1) +" Kg", 5, (plotCanvasHeight-(yDivisions* plotCanvasHeight / VScale) - 5));
       }                            //                                        ^ DAG note -1 gives a scale offset to match the  "64"  (equivalent of 1Kg) offset in the scales in scope commands
       else {
       pctx.fillText( (yDivisions / VScale * (yPlotMax) / yPlotScaleFactor).toFixed(1) +" V", 5, (plotCanvasHeight-(yDivisions* plotCanvasHeight / VScale) - 5));
       }                                                      
       }
            peakDetectFirstReadFlag = false;
            FirstAfterCLS=0;  
            PlotEdgeReached=false;
    }


    function updatePlot(incomingYPlotPosition)
    {
      if(!pauseScopeFlag)
      {
        var plotElementID = document.getElementById("plotElement");
        var pctx = plotElementID.getContext("2d");
        incomingYPlotPosition = ((incomingYPlotPosition / 4096) * yPlotMax);  //DAG NOTE 4096
        if(xPlotCurrentPosition > (plotCanvasWidth - 1)||(xPlotCurrentPosition2 > (plotCanvasWidth - 1)))
                   {
                      PlotEdgeReached=true;
                   }             
        pctx.lineWidth = plotCanvasHeight / 50;
        if(dataChannelOnFlag1 && (wsMessageArray[2] === "DATACHANNEL1"))
        { 
          if (FirstAfterCLS === 0){
        //    websock.send("CH1 rx first");
          FirstAfterCLS=1;
         // xPlotOldPosition2=(xPlotOldPosition2+ (xPlotPositionStep/2));          //offset other channel by half WS_Timer timestep  ???
         // xPlotCurrentPosition2=(xPlotCurrentPosition2+ (xPlotPositionStep/2)); //offset other channel by half WS_Timer timestep  ????
          }
          channelIncomingYPlotPosition1 = incomingYPlotPosition;
          
          xPlotOldPosition = xPlotCurrentPosition;
          xPlotCurrentPosition += xPlotPositionStep;
          yPlotOldPosition = yPlotCurrentPosition;
          yPlotCurrentPosition = channelIncomingYPlotPosition1 * yPlotScaleFactor;
          yPlotCurrentPosition = yPlotCurrentPosition * (plotCanvasHeight / yPlotMax);
          yPlotCurrentPosition = plotCanvasHeight - yPlotCurrentPosition;
      if (!PlotEdgeReached){
          pctx.strokeStyle = "#E87D75";
          pctx.beginPath();
          pctx.moveTo(xPlotOldPosition, yPlotOldPosition);
          if (PlotStraightLine)
           {pctx.moveTo(xPlotOldPosition, yPlotCurrentPosition);
           PlotStraightLine=false;}  // only do it first time
          pctx.lineTo(xPlotCurrentPosition, yPlotCurrentPosition);
          pctx.closePath();
          pctx.stroke();
          if(peakDetectionFlag)
          {
            pctx.fillStyle = "#E87D75";
            peakDetectInputValue = plotCanvasHeight - yPlotCurrentPosition;
            peakDetectFinder(peakDetectInputValue);
          }
        }
          if(dataOSDFlag)
          {
             if(document.getElementById("channelSelectElement1").value==="SCALES")
                  { OSDdisplay( (incomingYPlotPosition-1).toFixed(2) +" kG",0);} 
             else {  OSDdisplay( (incomingYPlotPosition).toFixed(2) +" V",0);}
           }
        }
     if(dataChannelOnFlag2 && (wsMessageArray[2] === "DATACHANNEL2"))
        { 
          if (FirstAfterCLS === 0){
          //  websock.send("CH2 first");
            FirstAfterCLS=2;
          xPlotOldPosition=(xPlotOldPosition+ (xPlotPositionStep/2))
          xPlotCurrentPosition=(xPlotCurrentPosition+ (xPlotPositionStep/2)); //offset other channel by half timestep
          }
          
          channelIncomingYPlotPosition2 = incomingYPlotPosition;
          
          xPlotOldPosition2 = xPlotCurrentPosition2;
          xPlotCurrentPosition2 += xPlotPositionStep
          yPlotOldPosition2 = yPlotCurrentPosition2;
          yPlotCurrentPosition2 = channelIncomingYPlotPosition2 * yPlotScaleFactor;
          yPlotCurrentPosition2 = yPlotCurrentPosition2 * (plotCanvasHeight / yPlotMax);
          yPlotCurrentPosition2 = plotCanvasHeight - yPlotCurrentPosition2;
          pctx.strokeStyle = "#7CCFF8";
          pctx.beginPath();
          pctx.moveTo(xPlotOldPosition2, yPlotOldPosition2);
          pctx.lineTo(xPlotCurrentPosition2, yPlotCurrentPosition2);
          pctx.closePath();
          pctx.stroke();
          if(peakDetectionFlag)
          {
            pctx.fillStyle = "#7CCFF8";
            peakDetectInputValue = plotCanvasHeight - yPlotCurrentPosition2;
            peakDetectFinder(peakDetectInputValue);
          }
          if(dataOSDFlag)
          {
             if((document.getElementById("channelSelectElement2").value==="SCALES")||(document.getElementById("channelSelectElement2").value==="SCALESB"))
                  { OSDdisplay( (incomingYPlotPosition-1).toFixed(2) +" kG",1);} 
             else {  OSDdisplay( (incomingYPlotPosition).toFixed(2) +" V",1);}
           }
        } // end ch 2
      }
    }

    function enterText()
    {
      if(event.keyCode == 13)
      {
        inputString = document.getElementById('txtSearch').value;
        if(inputString.localeCompare(""))
        {
          if(terminalEchoFlag)
          {
            var outputTable = document.getElementById('terminalOutput');
            var terminalOutputWrapperDiv = document.getElementById('terminalOutputWrapper');
            outputTable.innerHTML = outputTable.innerHTML + '<tr><td><span style="background-color:#4E4E56; color:white;">Echo</span> ' + inputString + '</tr><td>';
            terminalOutputWrapperDiv.scrollTop = terminalOutputWrapperDiv.scrollHeight;
          }
          websock.send(inputString);
          document.getElementById('txtSearch').value = "";
          inputString = "";
        }
        document.getElementById("mainBody").focus();
      }
      console.log("Code 13");
      websock.send(inputString);
      
    }

    function selectScope()
    {
      currentScreenElement = "SCOPE";
      updateButtonSelect(currentScreenElement);
    }

    function selectTerminal()
    {
      currentScreenElement = "TERMINAL";
      updateButtonSelect(currentScreenElement);
    }
    
    function selectStripChart()
    {
      currentScreenElement = "STRIPCHART";
      updateButtonSelect(currentScreenElement);

    }

    function selectI2C()
    {
      currentScreenElement = "I2C";
      updateButtonSelect(currentScreenElement);
      i2cFindDevices();
    }

    function updateButtonSelect(BUTTON)
    {
      toggleSettingsElementFlag = true;
      updateSettingsElementToggle();
      document.getElementById("setScopeButton").style.backgroundColor = "#E87D75";
      document.getElementById("setScopeButton").style.color = "white";
      document.getElementById("setTerminalButton").style.backgroundColor = "#E87D75";
      document.getElementById("setTerminalButton").style.color = "white";
      document.getElementById("setStripChartButton").style.backgroundColor = "#E87D75";
      document.getElementById("setStripChartButton").style.color = "white";

      document.getElementById("setI2CButton").style.backgroundColor = "#E87D75";
      document.getElementById("setI2CButton").style.color = "white";
      document.getElementById("oscilloscopeScreenElement").style.display = "none";
      document.getElementById("terminalScreenElement").style.display = "none";
      document.getElementById("i2cScreenElement").style.display = "none";
      if(BUTTON === "SCOPE")
      {
        document.getElementById("setScopeButton").style.backgroundColor = "white";
        document.getElementById("setScopeButton").style.color = "#4E4E56";
        document.getElementById("oscilloscopeScreenElement").style.display = "block";
      }
      if(BUTTON === "TERMINAL")
      {
        document.getElementById("setTerminalButton").style.backgroundColor = "white";
        document.getElementById("setTerminalButton").style.color = "#4E4E56";
        document.getElementById("terminalScreenElement").style.display = "block";
      }
      if(BUTTON === "STRIPCHART")
      {
        document.getElementById("setStripChartButton").style.backgroundColor = "white";
        document.getElementById("setStripChartButton").style.color = "#4E4E56";
        document.getElementById("terminalScreenElement").style.display = "block";
      }

      if(BUTTON === "I2C")
      {
        document.getElementById("setI2CButton").style.backgroundColor = "white";
        document.getElementById("setI2CButton").style.color = "#4E4E56";
        document.getElementById("i2cScreenElement").style.display = "block";
      }
    }

    function changeChannelSelect1()
    {
      channelSelect1 = "SCOPE CHANNEL 1 ";
      channelSelect1 += document.getElementById("channelSelectElement1").value;
      websock.send(channelSelect1);
      clearPlot(); // added
    }

    function changeChannelSelect2()
    {
      channelSelect2 = "SCOPE CHANNEL 2 ";
      channelSelect2 += document.getElementById("channelSelectElement2").value;
      websock.send(channelSelect2);
      clearPlot();  // added
    }

    function changeTimeScale()
    { 
      xPlotTotalTimeMax = document.getElementById("timescaleSelectElement").value;
        data2send="";
        data2send = "SCOPE WS_Timer ";
        data2send += (xPlotTotalTimeMax *1000);  // send ms
       websock.send(data2send);
       websock.send(" line 620 WS_Timer set xplottotal time ");
      clearPlot();
    }

    function changeYScale()
    {
      yPlotScaleFactor = (parseInt(document.getElementById("yScaleSelectElement").value));
      clearPlot();
    }

    function changeXScale()
    {
      xPlotScaleFactor = 1 / (parseInt(document.getElementById("xScaleSelectElement").value));
      clearPlot();
    }

    function terminalClear()
    {
      var outputTable = document.getElementById('terminalOutput');
      var terminalOutputWrapperDiv = document.getElementById('terminalOutputWrapper');
      outputTable.innerHTML = '<tr><td></tr><td>';
      terminalOutputWrapperDiv.scrollTop = terminalOutputWrapperDiv.scrollHeight;
    }

    function changeBaudRate()
    { 
      }

    function terminalConnect()
    {
      if(terminalConnectFlag)
      {
        terminalConnectFlag = false;
        document.getElementById("connectTerminalButton").innerHTML = "<b>Disconnected</b>";
        document.getElementById("connectTerminalButton").style.backgroundColor = "#E87D75";
        websock.send("TERMINAL CONNECT OFF");
      }
      else
      {
        terminalConnectFlag = true;
        document.getElementById("connectTerminalButton").innerHTML = "<b>Connected</b>";
        document.getElementById("connectTerminalButton").style.backgroundColor = "#4E4E56";
        websock.send("TERMINAL CONNECT ON");
      }
    }

    function togglePause()
    {
      if(pauseScopeFlag)
      {
        pauseScopeFlag = false;
        document.getElementById("togglePauseButton").innerHTML = "<b>Pause: Off</b>";
        document.getElementById("togglePauseButton").style.backgroundColor = "#E87D75";
        clearPlot();
      }
      else
      {
        pauseScopeFlag = true;
        document.getElementById("togglePauseButton").innerHTML = "<b>Pause: On</b>";
        document.getElementById("togglePauseButton").style.backgroundColor = "#4E4E56";
      }
    }

    function togglePeakDetection()
    {
      if(peakDetectionFlag)
      {
        peakDetectionFlag = false;
        document.getElementById("togglePeakDetectionButton").innerHTML = "<b>Peak Detection: Off</b>";
        document.getElementById("togglePeakDetectionButton").style.backgroundColor = "#E87D75";
      }
      else
      {
        peakDetectionFlag = true;
        document.getElementById("togglePeakDetectionButton").innerHTML = "<b>Peak Detection: On</b>";
        document.getElementById("togglePeakDetectionButton").style.backgroundColor = "#4E4E56";
      }
    }

    function peakDetectFinder(incomingData)
    {
      var plotElementID = document.getElementById("plotElement");
      var pctx = plotElementID.getContext("2d");
      pctx.fillStyle = "#4E4E56";
      if(!peakDetectFirstReadFlag)
      {
        peakDetectMinData = incomingData;
        peakDetectMaxData = incomingData;
        peakDetectMinMaxAverage = incomingData;
        peakDetectWindowScalar = 1;
        peakDetectTimeOffset = 1;
        peakDetectOnPeak = false;
        peakDetectDataWindowPos = 0;
        peakDetectDataWindowNeg = 0;
        peakDetectFirstReadFlag = true;
      }
      if(incomingData < peakDetectMinData)
      {
        peakDetectMinData = incomingData;
      }
      if(incomingData > peakDetectMaxData)
      {
        peakDetectMaxData = incomingData;
      }
      peakDetectMaxData *= peakDetectTimeOffset;
      peakDetectMinMaxAverage = (((peakDetectMaxData - peakDetectMinData) / 2) + peakDetectMinData);
      peakDetectDataWindowPos = (peakDetectMinMaxAverage + (peakDetectWindowScalar * (peakDetectMaxData / 16)));
      peakDetectDataWindowPosYValue = plotCanvasHeight - peakDetectDataWindowPos;
      pctx.beginPath();
      pctx.arc(xPlotCurrentPosition, peakDetectDataWindowPosYValue, (plotCanvasHeight / 150), 0, 2 * Math.PI, false);
      pctx.fill();
      peakDetectDataWindowNeg = (peakDetectMinMaxAverage - (peakDetectMinData / 16));
      peakDetectDataWindowNegYValue = plotCanvasHeight - peakDetectDataWindowNeg;
      pctx.beginPath();
      pctx.arc(xPlotCurrentPosition, peakDetectDataWindowNegYValue, (plotCanvasHeight / 150), 0, 2 * Math.PI, false);
      pctx.fill();
      peakDetectLastPeak = peakDetectOnPeak;
      if(incomingData > peakDetectDataWindowPos)
      {
        peakDetectOnPeak = true;
        peakDetectPeakXValue = xPlotCurrentPosition;
        peakDetectPeakYValue = incomingData;
        peakDetectTimeOffset = 1;
        peakDetectWindowScalar *= 1.2;
      }
      else if(incomingData < peakDetectDataWindowNeg)
      {
        peakDetectOnPeak = false;
        peakDetectTimeOffset *= 0.99;
        peakDetectWindowScalar *= 0.9;
      }
      else
      {
        peakDetectTimeOffset = 1;
      }
      if((peakDetectLastPeak) && (!peakDetectOnPeak) && (peakDetectPeakYValue > (plotCanvasHeight * 0.01)))
      {
        var plotElementID = document.getElementById("plotElement");
        var pctx = plotElementID.getContext("2d");
        peakDetectPeakYValue = plotCanvasHeight - peakDetectPeakYValue;
        pctx.beginPath();
        pctx.arc(peakDetectPeakXValue, peakDetectPeakYValue, (plotCanvasHeight / 20), 0, 2 * Math.PI, false);
        pctx.fill();
        peakDetectFirstReadFlag = false;
      }
    }

    function toggleDataLog()
    {
      if(dataLogFlag)
      {
        dataLogFlag = false;
        document.getElementById("toggleDataLogButton").innerHTML = "<b>Log Data: Off</b>"
        document.getElementById("toggleDataLogButton").style.backgroundColor = "#E87D75";
        websock.send("SCOPE DATALOG OFF");
      }
      else
      {
        dataLogFlag = true;
        document.getElementById("toggleDataLogButton").innerHTML = "<b>Log Data: On</b>"
        document.getElementById("toggleDataLogButton").style.backgroundColor = "#4E4E56";
        websock.send("SCOPE DATALOG ON");
      }
    }
     function toggleOSD()
    {
      if(dataOSDFlag)
      {
        dataOSDFlag = false;
        document.getElementById("toggleOSDButton").innerHTML = "<b> OSD: Off</b>"
        document.getElementById("toggleOSDButton").style.backgroundColor = "#E87D75";
        websock.send("SCOPE OSD OFF");
      }
      else
      {
        dataOSDFlag = true;
        document.getElementById("toggleOSDButton").innerHTML = "<b> OSD: On</b>"
        document.getElementById("toggleOSDButton").style.backgroundColor = "#4E4E56";
        websock.send("SCOPE OSD ON");
      }
    }
    function OSDdisplay(incomingData,Chan)
    { var plotElementID = document.getElementById("plotElement");   // dag to do change spacing to relate to canvas width
      var pctx = plotElementID.getContext("2d");
      var TextHeight = (plotCanvasHeight / 10);
      pctx.fillStyle = "white";
      pctx.clearRect((30+(Chan*plotCanvasWidth/5)), 10, 150, TextHeight);
      pctx.beginPath();
      pctx.fillRect((30+(Chan*plotCanvasWidth/5)),  10, 150, TextHeight);
      pctx.font = TextHeight + "px Helvetica";
      pctx.fillStyle = "black";
      //websock.send("OSD write");
      //pctx.font = "24px Helvitica";
      //pctx.fillText( "TEST", 30,30);
       pctx.fillText( incomingData, (30+(Chan*plotCanvasWidth/5)), (TextHeight+10) ,(plotCanvasWidth/5));
  
    }
    
    function toggleTerminalEcho()
    {
      if(terminalEchoFlag)
      {
        terminalEchoFlag = false;
        document.getElementById("toggleTerminalEchoButton").innerHTML = "<b>Echo: Off</b>"
        document.getElementById("toggleTerminalEchoButton").style.backgroundColor = "#E87D75";
      }
      else
      {
        terminalEchoFlag = true;
        document.getElementById("toggleTerminalEchoButton").innerHTML = "<b>Echo: On</b>"
        document.getElementById("toggleTerminalEchoButton").style.backgroundColor = "#4E4E56";
      }
    }

    function i2cClearDeviceField()
    {
      var i2cOutputDeviceWrapperDiv = document.getElementById('i2cOutputDeviceWrapper');
      i2cOutputDeviceWrapperDiv.innerHTML = '<button style="-webkit-appearance: none; width: 90%; height: 10vh; background-color:white; color:#4E4E56; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica;"><b>Devices On I2C Bus</b></button>';
      i2cOutputDeviceWrapperDiv.scrollTop = i2cOutputDeviceWrapperDiv.scrollHeight;
    }

    function i2cClearRegisterField()
    {
    lastI2CControlRegister = null;
    document.getElementById("i2cWriteField").placeholder = "Select a register to write";
      var i2cOutputRegisterWrapperDiv = document.getElementById('i2cOutputRegisterWrapper');
      i2cOutputRegisterWrapperDiv.innerHTML = '<button style="-webkit-appearance: none; width: 90%; height: 10vh; background-color:white; color:#4E4E56; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica;"><b>Internal Device Registers</b></button>';
      i2cOutputRegisterWrapperDiv.scrollTop = i2cOutputRegisterWrapperDiv.scrollHeight;
    }

    function i2cFindDevices()
    {
      i2cClearDeviceField();
      i2cClearRegisterField();
      websock.send("I2C FIND DEVICES");
    }

    function i2cRequestRegisterRead(deviceAddress)
    {
    if(lastI2CDeviceAddress != null)
    {
        document.getElementById(lastI2CDeviceAddress).style.backgroundColor = "#E87D75";
    }
      document.getElementById("i2cDevice"+deviceAddress).style.backgroundColor = "#4E4E56";
      i2cClearRegisterField();
    lastI2CDeviceAddress = "i2cDevice"+deviceAddress;
      var i2cAddressToRequest = "I2C READ DEVICE ";
      i2cAddressToRequest += deviceAddress;
      websock.send(i2cAddressToRequest);
    }
    
  function i2cUpdateControlRegister(controlRegister)
  {
    if(lastI2CControlRegister != null)
    {
        document.getElementById(lastI2CControlRegister).style.backgroundColor = "#E87D75";
    }
      document.getElementById("i2cRegister"+controlRegister).style.backgroundColor = "#4E4E56";
    lastI2CControlRegister = "i2cRegister"+controlRegister;
    document.getElementById("i2cWriteField").placeholder = "Write to register 0x"+controlRegister.toString(16).toUpperCase();
    
  }

    function updateSettingsElementToggle()
    {
      if(toggleSettingsElementFlag)
      {
        document.getElementById("setSettingsButton").style.backgroundColor = "#E87D75";
        document.getElementById("setSettingsButton").style.color = "white";
        document.getElementById("scopeSettingsElement").style.display = "none";
        document.getElementById("terminalSettingsElement").style.display = "none";
        document.getElementById("i2cSettingsElement").style.display = "none";
        if(currentScreenElement === "SCOPE")
        {
          document.getElementById("oscilloscopeScreenElement").style.display = "block";
          document.getElementById("oscilloscopeScreenElement").style.width = "85%";
        }
        if(currentScreenElement === "STRIPCHART")
        {
          //document.getElementById("StripChartScreenElement").style.display = "block";
          //document.getElementById("StripChartScreenElement").style.width = "85%";
        }
        if(currentScreenElement === "TERMINAL")
        {
          document.getElementById("terminalScreenElement").style.display = "block";
          document.getElementById("terminalScreenElement").style.width = "85%";
        }
        if(currentScreenElement === "I2C")
        {
          document.getElementById("i2cScreenElement").style.display = "block";
          document.getElementById("i2cScreenElement").style.width = "85%";
        }
        document.getElementById("settingsScreenElement").style.display = "none";
        toggleSettingsElementFlag = false;
      }
      else
      {
        document.getElementById("setSettingsButton").style.backgroundColor = "white";
        document.getElementById("setSettingsButton").style.color = "#4E4E56";
        if(currentScreenElement === "SCOPE")
        {
          document.getElementById("oscilloscopeScreenElement").style.display = "inline-block";
          document.getElementById("oscilloscopeScreenElement").style.width = "50%";
          document.getElementById("scopeSettingsElement").style.display = "block";
        }
        if(currentScreenElement === "TERMINAL")
        {
          document.getElementById("terminalScreenElement").style.display = "inline-block";
          document.getElementById("terminalScreenElement").style.width = "50%";
          document.getElementById("terminalSettingsElement").style.display = "block";
        }
        if(currentScreenElement === "I2C")
        {
          document.getElementById("i2cScreenElement").style.display = "inline-block";
          document.getElementById("i2cScreenElement").style.width = "50%";
          document.getElementById("i2cSettingsElement").style.display = "block";
        }
        document.getElementById("settingsScreenElement").style.display = "inline-block";
        toggleSettingsElementFlag = true;
      }
    }
	
	
  </script>
</head>

<body onload="javascript:start();" onresize="adjustCanvas();" id="mainBody" style="position:relative; overflow:hidden; width:98vw; height:90vh; margin:0; padding:0; margin-left:1vw; background-color:#4E4E56;" tabindex="1">
  <div id="toggleMenuElement" style="text-align:center; height: 10vh; margin-top: 2.5vh; margin-bottom: 2.5vh;">
    <div style="display:block; width: 85%; height: 70vh; text-align:center; margin-left:7.5%;"> 
      <button id="setScopeButton" style="-webkit-appearance: none; width: 15%; height: 10vh; background-color:white; color:#4E4E56; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica; margin-right:1%;" onclick="selectScope()">
        <b>Oscilloscope</b>
      </button><!--NOTE: This comment is to prevent white space between inline blocking elements.
     --><button id="setTerminalButton" style="-webkit-appearance: none; width: 15%; height: 10vh; background-color:#E87D75; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica; margin-left:1%; margin-right:1%;" onclick="selectTerminal()">
        <b>Terminal</b>
      </button><!--NOTE: This comment is to prevent white space between inline blocking elements.
    --><a href =/STRIP ><button  id="setStripChartButton" style="-webkit-appearance: none; width: 15%; height: 10vh; background-color:#E87D75; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica; margin-left:1%; margin-right:1%;" onclick="selectStripChart()">
        <b>StripChart</b> 
      </button></a><!--NOTE: This comment is to prevent white space between inline blocking elements.
     --><button id="setI2CButton" style="-webkit-appearance: none; width: 15%; height: 10vh; background-color:#E87D75; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica; margin-left:1%; margin-right:1%;" onclick="selectI2C()">
        <b>I2C</b>
      </button><!--NOTE: This comment is to prevent white space between inline blocking elements.
     --><button id="setSettingsButton" style="-webkit-appearance: none; width: 15%; height: 10vh; background-color:#E87D75; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica; margin-left:1%;" onclick="updateSettingsElementToggle()">
        <b>Settings</b>
      </button> </div>
  </div><!--NOTE: This comment is to prevent white space between inline blocking elements.
---><div id="oscilloscopeScreenElement" style="display:inline-block; vertical-align:top; text-align:center; margin-left:7.5%; width: 85%;"> <canvas id="plotElement" style="display: block; width:100%; height:77.5vh; border-radius: 5px;"></canvas> </div>
<!--NOTE: This comment is to prevent white space between inline blocking elements.--->
   <div id="terminalScreenElement" style="display:none; width: 85%; height: 80vh; margin-left:7.5%; font-family:Helvetica; vertical-align:top; text-align:center;">
    <div id="terminalOutputWrapper" style="height: 65vh; overflow:auto; background-color:white; border-radius: 5px;">
      <table id="terminalOutput" style="color:#4E4E56; text-align:left;">
        <tr>
          <td></td>
        </tr>
      </table>
    </div>
    <div style="width: 100%; height:10vh; margin-top:2.5vh; text-align:center;"> <input type="text" id="txtSearch" style="width: 100%; height:10vh; padding:0; border:0; border-radius: 5px; text-decoration: none; text-align:center;" autocomplete="off" autocorrect="off" autocapitalize="off" spellcheck="false" placeholder="Enter Text Here" onkeydown="enterText();" /> </div>
  </div>
  <!--NOTE: This comment is to prevent white space between inline blocking elements.
---><div id="i2cScreenElement" style="display:none; width: 85%; height: 80vh; margin-left:7.5%; font-family:Helvetica; vertical-align:top; text-align:center;">
    <div style="display:block; width:100%; height: 65vh; text-align:center;">
      <div id="i2cOutputDeviceWrapper" style="float:left; display: inline-block; width: 49%; height: 65vh; overflow:auto; background-color:white; border-radius: 5px; text-align:center;"> <button style="-webkit-appearance: none; width: 90%; height: 10vh; background-color:white; color:#4E4E56; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica;">
          <b>Devices On I2C Bus</b>
        </button></div><!--NOTE: This comment is to prevent white space between inline blocking elements.
     --><div id="i2cOutputRegisterWrapper" style="float:right; display: inline-block; width: 49%; height: 65vh; overflow:auto; background-color:white; border-radius: 5px; text-align:center;"> <button style="-webkit-appearance: none; width: 90%; height: 10vh; background-color:white; color:#4E4E56; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica;">
          <b>Internal Device Registers</b>
        </button> </div>
  </div>
  <div style="width: 100%; height:10vh; margin-top:2.5vh; text-align:center;"> 
    <input type="number" id="i2cWriteField" style="width: 100%; height:10vh; padding:0; border:0; border-radius: 5px; text-decoration: none; text-align:center;" autocomplete="off" autocorrect="off" autocapitalize="off" spellcheck="false" placeholder="Write to register"/>
  </div>
</div><!--NOTE: This comment is to prevent white space between inline blocking elements.
---><div id="settingsScreenElement" style="width: 33%; display:none; height: 77.5vh; font-family:Helvetica; vertical-align:top; text-align:center; background-color:white; color:#4E4E56; border-radius:5px; margin-left:2%;"><!--NOTE: This comment is to prevent white space between inline blocking elements.
  ---><div id="scopeSettingsElement" style="display:block; width:100%; height:77.5vh; overflow-y:auto; ">
      <div style="width: 100%; height:12.5vh; margin-top:2.5vh;"> <span style="width: 100%; height:2.5vh;">Pause</span> <button id="togglePauseButton" style="display:block; -webkit-appearance: none; width: 70%; height: 10vh; background-color: #E87D75; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica; margin-left:15%;" onclick="togglePause()">
        <b>Pause: Off</b>
      </button> </div>
      <div style="width: 100%; height:12.5vh; margin-top:2.5vh;"> <span style="width: 100%; height:2.5vh;">Channel 1</span> <select id="channelSelectElement1" onchange="changeChannelSelect1();" style="display:block; -webkit-appearance: none; box-sizing: content-box; width: 70%; height:10vh; background-color: #E87D75; color:white; border:0; border-radius: 5px; text-align:center; text-align-last:center; margin-left:15%;">
           <option value="OFF">Off</option>
           <option value="SCALES">HX711 Scales Ch_A</option> 
           <option value="INT ADC"selected="selected">Internal ADC (A0)</option> 
           <option value="DIG">Digital Input 1 </option>
           <option value="4V ADC" >4V ADC</option>
           <option value="64V ADC">64V ADC</option>
          
          <option value="UART">UART</option>
        </select> </div>
      <div style="width: 100%; height:12.5vh; margin-top:2.5vh;"> <span style="width: 100%; height:2.5vh;">Channel 2</span> <select id="channelSelectElement2" onchange="changeChannelSelect2();" style="display:block; -webkit-appearance: none; box-sizing: content-box; width: 70%; height:10vh; background-color: #E87D75; color:white; border:0; border-radius: 5px; text-align:center; text-align-last:center; margin-left:15%;">
          <option value="OFF"selected="selected">Off</option>
          <option value="SCALES">HX711 Scales Ch_A</option> 
          <option value="SCALESB">HX711 Scales Ch_B</option> 
          <option value="INT ADC">Internal ADC (A0)</option>
          <option value="DIG">Digital Input (2)</option>
          <option value="4V ADC">4V ADC</option>
          <option value="64V ADC">64V ADC</option>
          <option value="UART">UART</option>
        </select> </div>
      <div style="width: 100%; height:12.5vh; margin-top:2.5vh;"> <span style="width: 100%; height:2.5vh;">Signal Zoom</span> <select id="yScaleSelectElement" onchange="changeYScale();" style="display:block; -webkit-appearance: none; box-sizing: content-box; width: 70%; height:10vh; background-color: #E87D75; color:white; border:0; border-radius: 5px; text-align:center; text-align-last:center; margin-left:15%;">
          <option value="1" >1X</option>
          <option value="2">2X</option>
          <option value="5">5X</option>
          <option value="10" selected="selected">10X</option>
          <option value="20">20X</option>
          <option value="50">50X</option>
        </select> </div>
      <div style="width: 100%; height:12.5vh; margin-top:2.5vh;"> <span style="width: 100%; height:2.5vh;">Time Zoom</span> <select id="xScaleSelectElement" onchange="changeXScale();" style="display:block; -webkit-appearance: none; box-sizing: content-box; width: 70%; height:10vh; background-color: #E87D75; color:white; border:0; border-radius: 5px; text-align:center; text-align-last:center; margin-left:15%;">
          <option value="1" selected="selected">1X</option>
          <option value="2">2X</option>
          <option value="5">5X</option>
          <option value="10">10X</option>
          <option value="20">20X</option>
          <option value="100">100X</option>
          </select> </div>
      <div style="width: 100%; height:12.5vh; margin-top:2.5vh;"> <span style="width: 100%; height:2.5vh;">Max X Scale</span> <select id="timescaleSelectElement" onchange="changeTimeScale();" style="display:block; -webkit-appearance: none; box-sizing: content-box; width: 70%; height:10vh; background-color: #E87D75; color:white; border:0; border-radius: 5px; text-align:center; text-align-last:center; margin-left:15%;">
          <option value="1" >1s</option>
          <option value="2" >2s</option>
          <option value="10" selected="selected">10s</option>
          <option value="20">20s</option>
          <option value="30">30s</option>
          <option value="60">1min</option>
          <option value="300">5min</option>
          <option value="600">10min</option>
        </select> </div>
      <div style="width: 100%; height:12.5vh; margin-top:2.5vh;"> <span style="width: 100%; height:2.5vh;">Peak Detection</span> <button id="togglePeakDetectionButton" style="display:block; -webkit-appearance: none; width: 70%; height: 10vh; background-color: #E87D75; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica; margin-left:15%;" onclick="togglePeakDetection()">
        <b>Peak Detection: Off</b>
      </button> </div>
      <div style="width: 100%; height:12.5vh; margin-top:2.5vh; margin-bottom:2.5vh;"> <span style="width: 100%; height:2.5vh;">Log Data</span> <button id="toggleDataLogButton" style="display:block; -webkit-appearance: none; width: 70%; height: 10vh; background-color: #E87D75; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica; margin-left:15%;" onclick="toggleDataLog()">
        <b>Log Data: Off</b>
      </button> </div>
       <div style="width: 100%; height:12.5vh; margin-top:2.5vh; margin-bottom:2.5vh;"> <span style="width: 100%; height:2.5vh;">OSD</span> <button id="toggleOSDButton" style="display:block; -webkit-appearance: none; width: 70%; height: 10vh; background-color: #E87D75; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica; margin-left:15%;" onclick="toggleOSD()">
        <b>OSD: Off</b>
      </button> </div>
    </div><!--NOTE: This comment is to prevent white space between inline blocking elements.
  ---><div id="terminalSettingsElement" style="display:none; width:100%; height:77.5vh; overflow-y:auto; text-align:center; ">
      <div style="width: 100%; height:12.5vh; margin-top:2.5vh;"> <span style="width: 100%; height:2.5vh;">Connect</span> <button id="connectTerminalButton" style="display:block; -webkit-appearance: none; width: 70%; height: 10vh; background-color: #4E4E56; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica; margin-left:15%;" onclick="terminalConnect()">
        <b>Connected</b>
      </button> </div>
      <div style="width: 100%; height:12.5vh; margin-top:2.5vh"> <span style="width: 100%; height:2.5vh;">Baud Rate</span> <select id="channelSelectElement1" style="display:block; -webkit-appearance: none; box-sizing: content-box; width: 70%; height:10vh; background-color: #E87D75; color:white; border:0; border-radius: 5px; text-align:center; text-align-last:center; margin-left:15%;" onchange="changeBaudRate();">
        <option value="115200" selected="selected">115200</option>
        <option value="57600">57600</option>
        <option value="9600">9600</option>
      </select> </div>
      <div style="width: 100%; height:12.5vh; margin-top:2.5vh;"> <span style="width: 100%; height:2.5vh;">Clear Terminal</span> <button id="clearTerminalButton" style="display:block; -webkit-appearance: none; width: 70%; height: 10vh; background-color: #E87D75; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica; margin-left:15%;" onclick="terminalClear()">
        <b>Clear</b>
      </button> </div>
      <div style="width: 100%; height:12.5vh; margin-top:2.5vh; margin-bottom:2.5vh;"> <span style="width: 100%; height:2.5vh;">Local Echo</span> <button id="toggleTerminalEchoButton" style="display:block; -webkit-appearance: none; width: 70%; height: 10vh; background-color: #E87D75; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica; margin-left:15%;" onclick="toggleTerminalEcho()">
        <b>Echo: Off</b>
      </button> </div>
    </div><!--NOTE: This comment is to prevent white space between inline blocking elements.
  ---><div id="i2cSettingsElement" style="display:none; width:100%; height:77.5vh; overflow-y:auto; "> </div>
  </div> <iframe id="anchorFrameElement" style="display:none;" onload="javascript:adjustCanvas();"></iframe> </body>

</html>
)rawliteral";
