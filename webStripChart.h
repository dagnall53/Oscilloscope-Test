// https://www.cssscript.com/line-chart-picograph/
// https://github.com/RainingComputers/picograph.js



static const char PROGMEM STRIP_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <title> PicoGraphDemo </title>
 
 <script>
 // copied from Original picograph added PS in case of problems
var wsMessageArray = "";

var Data2,Data1;
var lastData2,lastData1;
var Test1 = [];
var Test2 = [];
var Data_Length = 0;
var Data_Updated = false;   
var xplotInterval= "2";
var pauseScopeFlag = false;

 var toggleSettingsElementFlag = false;
 var currentScreenElement = "STRIPCHART";



 var xPlotSampleRate = "3000";
 var WebSockTimeIntervalMS ="1000";
 var Scale1 = "38";  // 256/5
 var Scale2 = "100";  // 256/5

 var terminalEchoFlag = false;
 var dataLogFlag = false;
 var dataOSDFlag= false;
 var terminalConnectFlag = true;



    var inputString = "";
    var wsMessageArray = "";
    var i2cDeviceArray = [];
    var i2cRegisterArray = [];
    var lastI2CDeviceAddress = null;
    var lastI2CControlRegister = null;
   

    //*************I2C************ 
    function selectI2C()
    {
      currentScreenElement = "I2C";
      updateButtonSelect(currentScreenElement);
      i2cFindDevices();
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



    //************end I2C
    //******** terminal *********
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


    
 function enterText()
    { 
      console.log("enterTEXT() ECHO:%o  String is<%o>",terminalEchoFlag, document.getElementById('txtSearch').value);
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
          inputString = "SERIAL_OUT "+inputString;
          websock.send(inputString);
          document.getElementById('txtSearch').value = "";
          inputString = "";
        }
        document.getElementById("mainBody").focus();
      }
    }

    function selectTerminal()
    {
      currentScreenElement = "TERMINAL";
      updateButtonSelect(currentScreenElement);
    }

    function terminalClear()
    {
      var outputTable = document.getElementById('terminalOutput');
      var terminalOutputWrapperDiv = document.getElementById('terminalOutputWrapper');
      outputTable.innerHTML = '<tr><td></tr><td>';
      terminalOutputWrapperDiv.scrollTop = terminalOutputWrapperDiv.scrollHeight;
    }

    function changeBaudRate()
    {  _BaudRate = document.getElementById("baudSelectElement").value;
        data2send="";
        data2send = "SERIAL_BAUD ";
        data2send += (_BaudRate );  // send Baud
       websock.send(data2send);
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
          i2cOutputRegisterWrapperDiv.innerHTML = i2cOutputRegisterWrapperDiv.innerHTML + '<button id="i2cRegister'+parseInt(wsMessageArray[3], 16)+'"  style="-webkit-appearance: none; width: 90%; height: 3vh; margin-bottom:2%; background-color:#E87D75; color:white; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica;" onclick="i2cUpdateControlRegister(' + parseInt(wsMessageArray[3], 16) + ')"><b>Reading 0x' + wsMessageArray[3].toUpperCase() + ': 0x' + wsMessageArray[4].toUpperCase() + '</b></button>';
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





 
 
 function start()
    {
      //websock = new WebSocket('ws://192.168.4.1:81/');
      websock = new WebSocket('ws://' + window.location.hostname + ':81/');
      websock.onopen = function(evt)
      {
        console.log('websock open');
        websock.send("SCOPE DUPLEX 2  TRIANGLE");    // set ch2 = TRIANGLE  // note double space after 1 or 2 
        websock.send("SCOPE DUPLEX 1  INT ADC"); // set duplex and ch 1 = INT 
        websock.send("SCOPE Sample_uS 10000");  // dag note max rate ? about 3ms for internal
        websock.send("SCOPE WS_Timer 200");
        websock.send("Data_accepted "); // sets up flags
        websock.send("Clear_to_send ");   // initiates first websock send.. 
        websock.send("PicoGraph setup Finished "); 
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
            parseDuplexData();
            
          }
          if(wsMessageArray[0] === "SERIAL")
           {
           serialEventHandler();
          }
        }
      };
      
    }

function parseDuplexData() {
  // updatepoints replacement
 
 if(wsMessageArray[1] === "ADC") {
    if(wsMessageArray[2] === "DUPLEX") {  

      if(wsMessageArray.length > 3) { 
        Data_Length=0; 
				for(var Count = 3; Count <= (wsMessageArray.length-1); Count++)	{ 
          demograph.updatePoints( [ parseInt(wsMessageArray[Count])/parseInt(Scale1), parseInt(wsMessageArray[Count+1])/parseInt(Scale2) ] );
          Count++;
					Data_Length ++;
        
        }
        demograph.updateLegends( [ parseInt(wsMessageArray[Count-2])/parseInt(Scale1), parseInt(wsMessageArray[Count-3])/parseInt(Scale2) ]);
        demograph.updateTimestamps();
      }
    }
  console.log ( " parseDuplexData() reading:%d points",Data_Length );
  UpdateDisplay();
   
  } 


}

function UpdateDisplay(){
  // copy of .update, but without adding data..
console.log( "UPDATE DISPLAY");   
  demograph.clear()
  demograph.setWidthHeightAndCssScale()
        
  demograph.setStepAndFontSizePixels()
  demograph.setIntervalSizeAndLineWidth(2)
  if (demograph.vlines) {demograph.drawVerticalLines()}
  demograph.drawHorizontalLines()
  if (demograph.timestamps) {demograph.drawTimestamps()}
  demograph.drawGraph()
}

function selectStripChart()
    {
      currentScreenElement = "STRIPCHART";
      updateButtonSelect(currentScreenElement);
      resetStripChart();

    }

 function resetStripChart() {
        websock.send("SCOPE CHANNEL 2 DIG");    // set ch2 = dig  // set first!
        websock.send("SCOPE DUPLEX 1  INT ADC"); // set duplex and ch 1 = INT ADC
        websock.send("SCOPE Sample_uS 10000");  // dag note max rate ? about 10ms for internal
        
        websock.send("Data_accepted "); // Clears up flags
        websock.send("Clear_to_send ");   // should initiate first websock send.. 
        websock.send("PicoGraph setup Finished "); 
 }
 
 function changeSampleRate() {
        xPlotSampleRate = document.getElementById("xScaleSampleRateElement").value;
        data2send="";
        data2send = "SCOPE Sample_uS ";
        data2send += (xPlotSampleRate * 1000);  // send in us  CHANGE LATER to actual us 
       websock.send(data2send);
     }
     function changeInterval() {
        xPlotInterval = document.getElementById("IntervalSelectElement").value;
     }

function checkforSPSTiming( element){
  //console.log(" SPScheck "); console.log(element);
  if ( (element = "SCALES")|| (element = "SCALESB") ) {        // will need to check for other names as well ?
         xPlotSampleRate = 20;
         data2send="";
         data2send = "SCOPE Sample_uS ";
         data2send += (xPlotSampleRate * 1000);  // send in us  CHANGE LATER to actual us 
       websock.send(data2send);
       console.log("FORCING slower SPS %d ms for HX 711",xPlotSampleRate);
       }
  
 }

 function changeChannelSelect1()
    {
      channelSelect1 = "SCOPE DUPLEX 1  "; // note two spaces
      channelSelect1 += document.getElementById("channelSelectElement1").value;
      websock.send(channelSelect1);
      checkforSPSTiming(document.getElementById("channelSelectElement1").value);
    }

 function changeChannelSelect2()
    {
      channelSelect2 = "SCOPE DUPLEX 2  ";
      channelSelect2 += document.getElementById("channelSelectElement2").value;
      websock.send(channelSelect2);
      checkforSPSTiming(document.getElementById("channelSelectElement1").value);
      
    }
    

    function togglePause()
    {
      if(pauseScopeFlag)
      {
        pauseScopeFlag = false;
        websock.send("SCOPE PAUSE OFF");
        document.getElementById("togglePauseButton").innerHTML = "<b>Pause: Off</b>";
        document.getElementById("togglePauseButton").style.backgroundColor = "#E87D75";
        //clearPlot();
      }
      else
      { websock.send("SCOPE PAUSE ON");
        pauseScopeFlag = true;
        document.getElementById("togglePauseButton").innerHTML = "<b>Pause: On</b>";
        document.getElementById("togglePauseButton").style.backgroundColor = "#4E4E56";
      }
    }

 function SendWS() {
   WebSockTimeIntervalMS = document.getElementById("WSSendPeriodElement").value;
        data2send="";
        data2send = "SCOPE WS_Timer ";
        data2send += (WebSockTimeIntervalMS );  // send ms
       websock.send(data2send);
 }   
 
 function byID(id) {
    return document.getElementById(id)
}

function createValueIDs(labels, canvasID) {
    const valueIDs = []

    for (let i = 0; i < labels.length; i++)
        valueIDs[i] = canvasID + labels[i].replace(" ", "") + "value"

    return valueIDs
}
function createLegendRect(labelDivID, color, label, valueID) {
    const labelSpan = `<span>${label}</span>`
    const valueSpan = label.at(-1) == ":" ? `<span id="${valueID}"></span>` : ""

    byID(labelDivID).innerHTML += `
        <div style="display: inline-block;">
            <svg width="10" height="10">
                <rect width="10" height="10" style="fill: ${color}"/>
            </svg> 
            ${labelSpan}
            ${valueSpan}
        <div>
    `
}

function createGraph(
    canvasID,
    labels,
    unit,
    labelDivID,
    intervalSize = 3,
    maxVal = 10,
    minVal = 0,
    vlines = true,
    timestamps = true,
    scalesteps = 10,
    vlinesFreq = 50,
    autoScaleMode = 0
) {
    const valueIDs = createValueIDs(labels, canvasID)

    const graph = new Graph(
        canvasID,
        labels.length,
        valueIDs,
        unit,
        intervalSize,
        maxVal,
        minVal,
        vlines,
        timestamps,
        scalesteps,
        vlinesFreq,
        autoScaleMode
    )

    for (let i = 0; i < labels.length; i++)
        createLegendRect(labelDivID, graph.colors[i], labels[i] + ":", valueIDs[i])

    return graph
}

/* Graph class, plots and updates graphs */
class Graph {
    constructor(
        canvasID,
        noLabels,
        valueIDs,
        unit,
        intervalSize,
        maxVal,
        minVal,
        vlines,
        timestamps,
        scalesteps,
        vlinesFreq,
        autoScaleMode
    ) {
        this.canvas = byID(canvasID)
        this.ctx = this.canvas.getContext("2d")
        this.setWidthHeightAndCssScale()

        this.cssScale = window.devicePixelRatio
        this.intervalSize = intervalSize * this.cssScale

        this.nPointsFloat = this.width / this.intervalSize
        this.nPoints = Math.round(this.nPointsFloat) + 1
        this.points = emptyArray2D(noLabels, this.nPoints)
        this.maxVal = maxVal
        this.minVal = minVal

        this.noLabels = noLabels
        this.valueIDs = valueIDs
        this.unit = unit

        this.timestampsArray = emptyArray(this.nPoints, "")

        this.scalesteps = scalesteps
        this.vlinesFreq = vlinesFreq

        this.colors = colorArray(noLabels)

        this.fontSize = null
        this.hstep = null
        this.sstep = null

        this.vlines = vlines
        this.timestamps = timestamps
        this.autoScaleMode = autoScaleMode

        /* If autoscaling is disabled (or out of range) and min / max are not set, hard-set them to 0 and 100 respectively */
        if ([1, 2].includes(autoScaleMode) == false) {
            this.autoScaleMode = 0
            if (Number.isFinite(minVal) === false) this.minVal = 0
            if (Number.isFinite(maxVal) === false) this.maxVal = 100
        }
    }

    updateMinMax(value) {
        if (value < this.minVal) this.minVal = value

        if (value > this.maxVal) this.maxVal = value

        let valueMinMaxDelta = Math.abs(this.maxVal - this.minVal)

        let newMaxVal =
            Math.ceil(Math.max.apply(Math, this.points.flat().filter(Number.isFinite))) +
            valueMinMaxDelta * 0.05

        let newMinVal =
            Math.floor(Math.min.apply(Math, this.points.flat().filter(Number.isFinite))) -
            valueMinMaxDelta * 0.05

        if (this.autoScaleMode == 2) {
            this.maxVal = newMaxVal
            this.minVal = newMinVal
            return
        } else {
            /* autoScaleMode == 1 */
            if (value > this.maxVal - valueMinMaxDelta * 0.05) this.maxVal = newMaxVal

            if (value < this.minVal + valueMinMaxDelta * 0.05) this.minVal = newMinVal
        }
    }

    updatePoints(values) {
        for (let i = 0; i < this.noLabels; i++) {
            /* Update scale */
            if (this.autoScaleMode > 0) this.updateMinMax(values[i])

            /* Shift new point into points array */
            this.points[i] = shiftArrayLeft(this.points[i], values[i])
        }
    }

    updateLegends(values) {
     // console.log ( values )
        for (let i = 0; i < this.noLabels; i++)
            byID(this.valueIDs[i]).innerHTML = values[i].toFixed(2) + " " + this.unit
    }

    updateTimestamps() {
        const timestampString = getTimestamp()

        this.timestampsArray = shiftArrayLeft(this.timestampsArray, timestampString)
    }

    drawVerticalLines() {
        for (let i = this.nPoints - 1; i >= 0; i -= this.vlinesFreq) {
            /* Calculate line coordinates */
            const x = (i + 1) * this.intervalSize

            /* Draw line */
            this.ctx.beginPath()
            this.ctx.moveTo(x, 0)
            this.ctx.lineTo(x, this.height)
            this.ctx.strokeStyle = "#e3e3e3"
            this.ctx.stroke()
        }
    }

    clear() {
        this.ctx.clearRect(0, 0, this.width, this.height)
    }

    setWidthHeightAndCssScale() {
        this.cssScale = window.devicePixelRatio
        this.canvas.width = this.canvas.clientWidth * this.cssScale
        this.canvas.height = this.canvas.clientHeight * this.cssScale
        this.width = this.ctx.canvas.width
        this.height = this.ctx.canvas.height
    }

    setIntervalSizeAndLineWidth(value) {
        this.intervalSize = this.width / this.nPointsFloat
        this.ctx.lineWidth = value * this.cssScale
    }

    setStepAndFontSizePixels() {
        this.hstep = this.height / this.scalesteps
        this.sstep = (this.maxVal - this.minVal) / this.scalesteps
        this.fontSize = Math.min(0.5 * this.hstep, 15 * this.cssScale)
        this.ctx.font = this.fontSize + "px monospace"
    }

    drawHorizontalLines() {
        let entityDecode = document.createElement("textarea")
        entityDecode.innerHTML = this.unit

        for (let i = 1; i <= this.scalesteps; i++) {
            const y = this.height - i * this.hstep
            const xoffset = 2
            const yoffset = this.fontSize + 2 * this.cssScale
            this.ctx.fillText(
                (i * this.sstep + this.minVal).toFixed(2) + " " + entityDecode.value,
                xoffset,
                y + yoffset
            )
            this.ctx.beginPath()
            this.ctx.moveTo(0, y)
            this.ctx.lineTo(this.width, y)
            this.ctx.strokeStyle = "#e3e3e3"
            this.ctx.stroke()
        }
    }

    drawTimestamps() {
        const xBoundPix = this.ctx.measureText((this.scalesteps * this.sstep).toFixed(2)).width
        const xBound = Math.floor(xBoundPix / this.intervalSize + 1)

        for (let i = this.nPoints - 1; i >= xBound; i -= this.vlinesFreq) {
            /* Calculate line coordinates */
            const x = (i + 1) * this.intervalSize

            /* Put time stamps */
            const xoffset = this.fontSize + 2 * this.cssScale
            const yoffset = this.ctx.measureText(this.timestampsArray[i]).width + 4 * this.cssScale
            this.ctx.rotate(Math.PI / 2)
            this.ctx.fillText(this.timestampsArray[i], this.height - yoffset, -x + xoffset)
            this.ctx.stroke()
            this.ctx.rotate(-Math.PI / 2)
        }
    }

    drawGraph() {
      this.ctx.lineWidth = 8;
        for (let i = 0; i < this.noLabels; i++) {
            for (let j = this.nPoints - 1; j > 0; j--) {
                /* Calculate line coordinates */
                const xstart = j * this.intervalSize
                const xend = (j - 1) * this.intervalSize
                const ystart = scaleInvert(this.points[i][j], this.minVal, this.maxVal, this.height)
                const yend = scaleInvert(
                    this.points[i][j - 1],
                    this.minVal,
                    this.maxVal,
                    this.height
                )

                /* Draw line */
                this.ctx.beginPath()
                this.ctx.moveTo(xstart, ystart)
                this.ctx.lineTo(xend, yend)
                this.ctx.strokeStyle = this.colors[i]
                this.ctx.stroke()
            }
        }
    }

    update(values) {
        this.updatePoints(values)
        this.updateLegends(values)
        this.updateTimestamps()

        this.clear()
        this.setWidthHeightAndCssScale()
        
        this.setStepAndFontSizePixels()
        this.setIntervalSizeAndLineWidth(2)
        if (this.vlines) this.drawVerticalLines()
        this.drawHorizontalLines()

        if (this.timestamps) this.drawTimestamps()
        this.drawGraph()
    }
}

function getTimestamp() {
    const d = new Date()

    const timestampString =
        (d.getHours() < 10 ? "0" : "") +
        d.getHours() +
        (d.getMinutes() < 10 ? ":0" : ":") +
        d.getMinutes() +
        (d.getSeconds() < 10 ? ":0" : ":") +
        d.getSeconds() + ":"+
        d.getMilliseconds() 
        

    return timestampString
}

/* Helper function to take a value value and return y-coordinate for the canvas */
function scaleInvert(value, minVal, maxVal, height) {
    return (1 - (value - minVal) / (maxVal - minVal)) * height
}

function shiftArrayLeft(array, newVal) {
    array.shift()
    array.push(newVal)
    return array
}

function emptyArray2D(nrows, ncols, fill = undefined) {
    return Array.from({ length: nrows }, (_) => Array.from({ length: ncols }, (_) => fill))
}

function emptyArray(length, fill = undefined) {
    return Array.from({ length }, (_) => fill)
}

const colors = [
    "#e52b50",
    "#008000",
    "#0000ff",
    "#ff00ff",
    "#a52a2a",
    "#00008b",
    "#008b8b",
    "#a9a9a9",
    "#006400",
    "#bdb76b",
    "#8b008b",
    "#556b2f",
    "#ff8c00",
    "#9932cc",
    "#8b0000",
    "#e9967a",
    "#9400d3",
    "#ff00ff",
    "#ffd700",
    "#008000",
    "#4b0082",
    "#f0e68c",
    "#add8e6",
    "#e0ffff",
    "#90ee90",
    "#d3d3d3",
    "#ffb6c1",
    "#ffffe0",
    "#800000",
    "#000080",
    "#808000",
    "#ffa500",
    "#ffc0cb",
    "#800080",
    "#800080",
    "#ff0000",
    "#c0c0c0",
    "#ffffff",
    "#ffff00",
]

function colorArray(len) {
    const colorArray = []

    for (let i = 0; i < len; i++) colorArray[i] = colors[i % colors.length]

    return colorArray
}


 function updateButtonSelect(BUTTON) // may need changing for strip chart
 {
    console.log(" updateButtonSelect"); console.log (BUTTON);
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
        document.getElementById("oscilloscopeScreenElement").style.display = "block";
      }

      if(BUTTON === "I2C")
      {
        document.getElementById("setI2CButton").style.backgroundColor = "white";
        document.getElementById("setI2CButton").style.color = "#4E4E56";
        document.getElementById("i2cScreenElement").style.display = "block";
      }
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
        if(currentScreenElement === "STRIPCHART")
        {
          document.getElementById("oscilloscopeScreenElement").style.display = "block";
          document.getElementById("oscilloscopeScreenElement").style.width = "85%";
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
        if(currentScreenElement === "STRIPCHART")
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
	
  function updateEverySecond() {
        
        if(!pauseScopeFlag){ 
          console.log (" every second");
          console.log(Data_Length);       
            if (Data_Updated == true){
 	            for(var Counter = 0; Counter <= Data_Length-1; Counter++)	{ 
               demograph.update([Test1[Counter], Test2[Counter]]);
               } 
             Data_Length = 0;
             Data_Updated = false;
             websock.send("Clear_to_send "); 
            } else 
            {console.log (" No Data ");websock.send("Clear_to_send ");} 
        } 
         
      }





 </script>




</head>
<style> 
.SetBoxStyle {
    -webkit-appearance: none;
    width: 18%;
    height: 8vh;
    background-color: #E87D75;
    color: white;
    text-decoration: none;
    border: 0;
    padding: 0;
    border-radius: 5px;
    font-family: Helvetica;
    margin-left: 1%;
    margin-right: 1%;
    font-size: 25px;
}
.VertBoxStyle{
    display: block;
    -webkit-appearance: none;
    width: 70%;
    height: 4vh;
    background-color: #E87D75;
    color: white;
    text-decoration: none;
    border: 0;
    padding: 0;
    border-radius: 5px;
    font-family: Helvetica;
    margin-left: 15%;
    font-size: 18px;
}
.VertSelectBox{
  width: 100%;
  height: 6vh;
  margin-bottom: 2vh;
  margin-top: 2vh;
  font-size: 15px;
}
 .SettingsTitle{
   width:100%;
   height: 2.5vh;
   font-size: 20px;
 }


</style>



<body onload="javascript:start();" id="mainBody" style="font-family: Lucida Console, Monaco, monospace; background-color: Gray;">
<div id="toggleMenuElement" style="text-align:center; height: 10vh; margin-top: 2.5vh; margin-bottom: 2.5vh;">
    <div style="display:block; width: 85%; height: 70vh; text-align:center; margin-left:7.5%;"> 
      <a href =/HOME ><button id="setScopeButton" class="SetBoxStyle" onclick="selectScope()">
        <b>Oscilloscope</b>
      </button></a><!--NOTE: This comment is to prevent white space between inline blocking elements.
     --><button id="setTerminalButton" class="SetBoxStyle" onclick="selectTerminal()">
        <b>Terminal</b>
      </button><!--NOTE: This comment is to prevent white space between inline blocking elements.
    --><button  id="setStripChartButton" class="SetBoxStyle" style="background-color:white; color:black !important" onclick="selectStripChart()">
        <b>StripChart</b> 
      </button><!--NOTE: This comment is to prevent white space between inline blocking elements.
     --><button id="setI2CButton" class="SetBoxStyle" onclick="selectI2C()">
        <b>I2C</b>
      </button><!--NOTE: This comment is to prevent white space between inline blocking elements.
     --><button id="setSettingsButton" class="SetBoxStyle" onclick="updateSettingsElementToggle()">
        <b>Settings</b>
      </button> 
      </div>
  </div><!--NOTE: This comment is to prevent white space between inline blocking elements.-->
    <div id="graphLabels" >   Extra text to be added here later: id= "Data_Length"  </div></div>
    <!-- div for legends/labels -->
  <div id="oscilloscopeScreenElement" style="display:inline-block; vertical-align:top; text-align:center; margin-left:7.5%; width: 85%;">
    <!-- Canvas for the graph also click to get to previous page -->
    <canvas
        id="graphDemo"
        style="width: 100%; height:70vh; border:2px solid #000000;"
    >
    </canvas> 
    </div>
    <!-- div for legends/labels -->
   
    <!-- // NEW STUFF-->
    <!--NOTE: This comment is to prevent white space between inline blocking elements.--->
   <div id="terminalScreenElement" style="display:none; width: 85%; height: 80vh; margin-left:7.5%; font-family:Helvetica; vertical-align:top; text-align:center;">
    <div id="terminalOutputWrapper" style="height: 65vh; overflow:auto; background-color:white; border-radius: 5px;">
      <table id="terminalOutput" style="color:#4E4E56; text-align:left;">
        <tr>
          <td></td>
        </tr>
      </table>
    </div>
    <div style="width: 100%; height:10vh; margin-top:2.5vh; text-align:center;"> 
       <input type="text" id="txtSearch" style="width: 100%; height:10vh; padding:0; border:0; border-radius: 5px; text-decoration: none; text-align:center;" autocomplete="off" autocorrect="off" autocapitalize="off" spellcheck="false" placeholder="Enter Text Here" onkeydown="enterText();" /> </div>
  </div>
  <!--NOTE: This comment is to prevent white space between inline blocking elements.--->
  <div id="i2cScreenElement" style="display:none; width: 85%; height: 80vh; margin-left:7.5%; font-family:Helvetica; vertical-align:top; text-align:center;">
    <div style="display:block; width:100%; height: 65vh; text-align:center;">
      <div id="i2cOutputDeviceWrapper" style="float:left; display: inline-block; width: 49%; height: 65vh; overflow:auto; background-color:white; border-radius: 5px; text-align:center;"> <button style="-webkit-appearance: none; width: 90%; height: 10vh; background-color:white; color:#4E4E56; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica;">
          <b>Devices On I2C Bus</b>
        </button></div>
        <!--NOTE: This comment is to prevent white space between inline blocking elements. -->
          <div id="i2cOutputRegisterWrapper" style="float:right; display: inline-block; width: 49%; height: 65vh; overflow:auto; background-color:white; border-radius: 5px; text-align:center;"> <button style="-webkit-appearance: none; width: 90%; height: 10vh; background-color:white; color:#4E4E56; text-decoration: none; border: 0; padding: 0; border-radius: 5px; font-family:Helvetica;">
          <b>Internal Device Registers</b>
        </button> </div>
  </div>
  <div style="width: 100%; height:10vh; margin-top:2.5vh; text-align:center;"> 
    <input type="number" id="i2cWriteField" style="width: 100%; height:10vh; padding:0; border:0; border-radius: 5px; text-decoration: none; text-align:center;" autocomplete="off" autocorrect="off" autocapitalize="off" spellcheck="false" placeholder="Write to register"/>
  </div>
</div>
<div id="settingsScreenElement" style="width: 33%; display:none; height: 77.5vh; font-family:Helvetica; vertical-align:top; text-align:center; background-color:white; color:#4E4E56; border-radius:5px; margin-left:2%;"><!--NOTE: This comment is to prevent white space between inline blocking elements.
  ---><div id="scopeSettingsElement" style="display:block; width:100%; height:77.5vh; overflow-y:auto; ">
      <div class= "VertSelectBox"  > <span class= "SettingsTitle" >Pause</span> <button id="togglePauseButton" class="VertBoxStyle" onclick="togglePause()">
        <b>Pause: Off</b>
      </button> </div>
      <div class= "VertSelectBox" > <span class= "SettingsTitle" >Channel 1</span> <select id="channelSelectElement1" class="VertBoxStyle" onchange="changeChannelSelect1();" >
           <option value="OFF">Off</option>
           <option value="SCALES">HX711 Scales Ch_A</option> 
           <option value="INT ADC"selected="selected">Internal ADC (A0)</option> 
           <option value="DIG">Digital Input 1 </option>
           <option value="4V ADC" >4V ADC</option>
           <option value="64V ADC">64V ADC</option>
          
          <option value="UART">UART</option>
        </select> </div>
      <div class= "VertSelectBox" > <span class= "SettingsTitle" >Channel 2</span> <select id="channelSelectElement2" class="VertBoxStyle" onchange="changeChannelSelect2();" >
          <option value="OFF">Off</option>
          <option value="SCALES">HX711 Scales Ch_A</option> 
          <option value="SCALESB">HX711 Scales Ch_B</option> 
          <option value="INT ADC">Internal ADC (A0)</option>
          <option value="DIG" >Digital Input (2)</option>
          <option value="4V ADC">4V ADC</option>
          <option value="64V ADC">64V ADC</option>
          <option value="TRIANGLE" selected="selected">Triangle test</option>
        </select> </div>
      <div class= "VertSelectBox" > <span class= "SettingsTitle" >Sample Rate (ms)</span> <select id="xScaleSampleRateElement" class="VertBoxStyle" onchange="changeSampleRate();" >
          <option value="1" >1ms fastest</option>
          <option value="3" >3ms </option>
          <option value="5" >5ms </option>
          <option value="10" selected="selected" >10ms</option>
          <option value="20">20ms</option>
          <option value="100">100ms</option>
          <option value="1000" >1sec</option>
          <option value="10000">10sec</option>
          
        </select> </div>
      <div class= "VertSelectBox" > <span  class= "SettingsTitle" >WS Send Period </span> <select id="WSSendPeriodElement" class="VertBoxStyle" onchange="SendWS();" >
          <option value="50"> 50ms </option>
          <option value="100">100ms</option>
          <option value="500">0.5s</option>
          <option value="1000" selected="selected">1s</option>
          <option value="10000">10s</option>
         
          </select> </div>
      <div class= "VertSelectBox" > <span  class= "SettingsTitle" >Interval</span> <select id="IntervalSelectElement" class="VertBoxStyle" onchange="changeInterval();" >
          <option value="1" >1</option>
          <option value="2" >2</option>
           <option value="3" >3</option>
          <option value="5" selected="selected">5</option>
          <option value="10">10</option>
          <option value="20">20</option>
        </select> </div>
      <div class= "VertSelectBox" > <span  class= "SettingsTitle" >Peak Detection</span> <button id="togglePeakDetectionButton" class="VertBoxStyle"  onclick="togglePeakDetection()">
        <b>Peak Detection: Off</b>
      </button> </div>
      <div class= "VertSelectBox" > <span  class= "SettingsTitle" >Log Data</span> <button id="toggleDataLogButton" class="VertBoxStyle"  onclick="toggleDataLog()">
        <b>Log Data: Off</b>
      </button> </div>
       <div class= "VertSelectBox" > <span  class= "SettingsTitle" >OSD</span> <button id="toggleOSDButton" class="VertBoxStyle"  onclick="toggleOSD()">
        <b>OSD: Off</b>
      </button> </div>
    </div><!--NOTE: This comment is to prevent white space between inline blocking elements.
  ---><div id="terminalSettingsElement" style="display:none; width:100%; height:77.5vh; overflow-y:auto; text-align:center; ">
      <div class= "VertSelectBox" > <span  class= "SettingsTitle" >Connect</span> <button id="connectTerminalButton" class="VertBoxStyle"  onclick="terminalConnect()">
        <b>Connected</b>
      </button> </div>
      <div class= "VertSelectBox" > <span  class= "SettingsTitle" >Baud Rate</span> <select id="baudSelectElement" class="VertBoxStyle"  onchange="changeBaudRate();">
        <option value="115200" selected="selected">115200</option>
        <option value="57600">57600</option>
        <option value="9600">9600</option>
      </select> </div>
      <div class= "VertSelectBox" > <span  class= "SettingsTitle" >Clear Terminal</span> <button id="clearTerminalButton" class="VertBoxStyle"  onclick="terminalClear()">
        <b>Clear</b>
      </button> </div>
      <div style="width: 100%; height:12.5vh; margin-top:2.5vh; margin-bottom:2.5vh;"> <span style="width: 100%; height:2.5vh;">Local Echo</span> <button id="toggleTerminalEchoButton" class="VertBoxStyle"  onclick="toggleTerminalEcho()">
        <b>Echo: Off</b>
      </button> </div>
    </div>
    <!--NOTE: This comment is to prevent white space between inline blocking elements. --->
    <div id="i2cSettingsElement" style="display:none; width:100%; height:77.5vh; overflow-y:auto; "> </div>
  </div>
    <script>
        /* Create graph using picograph   */
        let demograph = createGraph("graphDemo",
            ["CH1", "CH2"],
            " ", "graphLabels"  );

        // * Run this at very xxxx ms to run demograph update*/
       //setInterval(updateEverySecond,1000); 
       //setInterval(UpdateDisplay,1000); // notneeded, built into parseDuplexdata
    


    </script>
</body>

</html>
)rawliteral";
