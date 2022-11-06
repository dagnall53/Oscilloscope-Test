#ifndef _OTA_WEB_H_
#define _OTA_WEB_H_


const char OTA_STYLE[] PROGMEM = "<style>#file-input,input{width:100%;height:35px;border-radius:4px;margin:10px auto;font-size:15px}\r\n"
"#file-input{padding:0;border:1px solid #0F458D;line-height:35px;text-align:center;display:block;cursor:pointer}\r\n"
"#bar{background-color:#0F458D;width:0%;border-radius:3px;height:15px}\r\n"
"form{max-width:258px;margin:5px auto;padding:30px;border-radius:5px;text-align:center}\r\n"
".btn{background:#0F458D;color:#fff;cursor:pointer}</style>";

const char OTA_START[] PROGMEM = "<br><br><div class=\"div2\">\r\n"
 "Click  CHOOSE FILE ...  Then press  UPLOAD<br>\r\n"
 "THEN wait until the progress bar reaches 100%. <br>You still need to wait until you receive a Success\r\n"
 "Message. <br>When that happens the unit has self reset and you need to connect to the new created Wifi\r\n"
 "network and check or set your settings<br><a href=\"/.\">Click this text to return to previous screen</a><br>\r\n"; 

const char OTA_UPLOAD[] PROGMEM = R"=+=+(
<form method="POST" action="ota" enctype="multipart/form-data" id="upload_form">
   <input type="file" name="update" id="file" onchange="sub(this)" style=display:none>
   <label id="file-input" for="file">CHOOSE FILE ...</label>
    <input type="submit" class=btn value="UPLOAD">
    <div id="prg"></div>
    <div id="bar"></div>
</form>
<script>
function sub(obj){
  var fileName = obj.value.split("\\\\");
  document.getElementById("file-input").innerHTML = " " + fileName[fileName.length-1];
};
var domReady = function(callback) {
  document.readyState === "interactive" || document.readyState === "complete" ? callback() : document.addEventListener("DOMContentLoaded", callback);
};
domReady(function() {
  var myform = document.getElementById('upload_form');
  var filez  = document.getElementById('file');
  myform.onsubmit = function(event) {
    event.preventDefault();
    var formData = new FormData();
    var file     = filez.files[0];
    if (!file) { return false; }
    formData.append("files", file, file.name);
    var xhr = new XMLHttpRequest();
    xhr.upload.addEventListener("progress", function(evt) {
      if (evt.lengthComputable) {
        var per = Math.round((evt.loaded / evt.total) * 100);
        var prg = document.getElementById('prg');
        prg.innerHTML     ="PROGRESS:" + per + "%"
        var bar = document.getElementById('bar');
        bar.style.width   = per + "%"
      }
    }, false);
    xhr.open('POST', location.href, true);
    // Set up a handler for when the request finishes.
    xhr.onload = function () {
      if (xhr.status === 200) {
        alert('Success!');
      } else {
        alert('An error occurred!');
      }
    };
    xhr.send(formData);
   }
});
</script>)=+=+";


#endif
