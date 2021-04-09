const char index_main[] PROGMEM = R"=====(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Alexa Face Recognition</title>
<style>
@media only screen and (min-width: 850px) {
  body {
     display: flex;
  }
   #content-right {
     margin-left: 10px;
  }  
}
body {
     font-family: Arial, Helvetica, sans-serif;
     background: #181818;
     color: #EFEFEF;
     font-size: 16px;
}
 #content-left {
     max-width: 400px;
   flex: 1;
}
 #content-right {
     max-width: 400px;
   flex: 1;
}
 #stream {
     width: 100%;
}
 #status-display {
     height: 25px;
     border: none;
     padding: 10px;
     font: 18px/22px sans-serif;
     margin-bottom: 10px;
     border-radius: 5px;
     background: green;
     text-align: center;
}
 #person {
     width:100%;
     height: 25px;
     border: none;
     padding: 20px 10px;
     font: 18px/22px sans-serif;
     margin-bottom: 10px;
     border-radius: 5px;
     resize: none;
     box-sizing: border-box;
}
 button {
     display: block;
     margin: 5px 0;
     padding: 0 12px;
     border: 0;
     width: 48%;
     line-height: 28px;
     cursor: pointer;
     color: #fff;
     background: #ff3034;
     border-radius: 5px;
     font-size: 16px;
     outline: 0;
}
 .buttons {
     height:40px;
}
 button:hover {
     background: #ff494d;
}
 button:active {
     background: #f21c21;
}
 button:disabled {
     cursor: default;
     background: #a0a0a0;
}
 .left {
     float: left;
}
 .right {
     float: right;
}
 .image-container {
     position: relative;
}
 .stream {
     max-width: 400px;
}
 ul {
     list-style: none;
     padding: 5px;
     margin:0;
}
 li {
     padding: 5px 0;
}
 .delete {
     background: #ff3034;
     border-radius: 100px;
     color: #fff;
     text-align: center;
     line-height: 18px;
     cursor: pointer;
}
 h3 {
     margin-bottom: 3px;
}
</style>
</head>
<body>
<div id="content-left">
  <div id="stream-container" class="image-container"> <img id="stream" src=""> </div>
</div>
<div id="content-right">
  <div id="status-display"> <span id="current-status"></span> </div>
  <div id="person-name">
    <input id="person" type="text" value="" placeholder="Type the person's name here">
  </div>
  <div class="buttons">
    <button id="button-stream" class="left">STREAM CAMERA</button>
    <button id="button-detect" class="right">DETECT FACES</button>
  </div>
  <div class="buttons">
    <button id="button-capture" class="left" title="Enter a name above before capturing a face">ADD USER</button>
    <button id="button-recognise" class="right">DETECT PERSON</button>
  </div>
  <div class="people">
    <h3>Captured Faces</h3>
    <ul>
    </ul>
  </div>
  <div class="buttons">
    <button id="delete_all">DELETE ALL</button>
  </div>
</div>
<script>
document.addEventListener("DOMContentLoaded", function(event) {
  const WS_URL = "ws://" + window.location.hostname + ":81";
  const ws = new WebSocket(WS_URL);

  const view = document.getElementById("stream");
  const personFormField = document.getElementById("person");
  const streamButton = document.getElementById("button-stream");
  const detectButton = document.getElementById("button-detect");
  const captureButton = document.getElementById("button-capture");
  const recogniseButton = document.getElementById("button-recognise");
  const deleteAllButton = document.getElementById("delete_all");

  ws.onopen = () => {
    console.log("Connected to ${WS_URL}");
  };
  ws.onmessage = message => {
    if (typeof message.data === "string") {
      if (message.data.substr(0, 8) == "listface") {
        addFaceToScreen(message.data.substr(9));
      } else if (message.data == "delete_faces") {
        deleteAllFacesFromScreen();
      } else {
          document.getElementById("current-status").innerHTML = message.data;
          document.getElementById("status-display").style.background = "green";
      }
    }
    if (message.data instanceof Blob) {
      var urlObject = URL.createObjectURL(message.data);
      view.src = urlObject;
    }
  };

  streamButton.onclick = () => {
    ws.send("stream");
  };
  detectButton.onclick = () => {
    ws.send("detect");
  };
  captureButton.onclick = () => {
    person_name = document.getElementById("person").value;
    ws.send("capture:" + person_name);
  };
  recogniseButton.onclick = () => {
    ws.send("recognise");
  };
  deleteAllButton.onclick = () => {
    ws.send("delete_all");
  };
  personFormField.onkeyup = () => {
    captureButton.disabled = false;
  };

  function deleteAllFacesFromScreen() {
    // deletes face list in browser only
    const faceList = document.querySelector("ul");
    while (faceList.firstChild) {
      faceList.firstChild.remove();
    }
    personFormField.value = "";
    captureButton.disabled = true;
  }

  function addFaceToScreen(person_name) {
    const faceList = document.querySelector("ul");
    let listItem = document.createElement("li");
    let closeItem = document.createElement("span");
    closeItem.classList.add("delete");
    closeItem.id = person_name;
    closeItem.addEventListener("click", function() {
      ws.send("remove:" + person_name);
    });
    listItem.appendChild(
      document.createElement("strong")
    ).textContent = person_name;
    listItem.appendChild(closeItem).textContent = "X";
    faceList.appendChild(listItem);
  }

  captureButton.disabled = true;
});
</script>
</body>
</html>
)=====";
