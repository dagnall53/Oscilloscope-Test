
// https://www.cssscript.com/line-chart-picograph/
// https://github.com/RainingComputers/picograph.js

static const char PROGMEM STRIP_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <title> PicoGraphDemo</title>
    <script src="picograph.js"></script>
</head>

<body style="font-family: Lucida Console, Monaco, monospace;">

    <h1>PicoGraphDemo</h1>
<a href=/DATA >
    <!-- Canvas for the graph also click to get to DATA page -->
    <canvas
        id="graphDemo"
        style="width: 900px; height:200px; border:2px solid #000000;"
    >
    </canvas>
</a>
    <!-- div for legends/labels -->
    <div id="graphLabels"></div>

    <script>
        /* Create graph using picograph */
        let demograph = createGraph("graphDemo",
            ["Random Y0", "Random Y1"],
            "units", "graphLabels", 50, 10, 0, true, true);

        /* Update values every second */
        setInterval(updateEverySecond, 1000);

        function updateEverySecond() {
            /* Get new values */
            yrand0 = Math.random() * 10;
            yrand1 = Math.random() * 10;

            /* Update graph */
            demograph.update([yrand0, yrand1])
        }

    </script>
</body>

</html>
)rawliteral";
