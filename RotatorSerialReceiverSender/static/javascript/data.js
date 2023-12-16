var socket = io.connect()

socket.on("send_data", (gps_lat, gps_lng, gps_alt, baro_alt, speed, temperature, rssi, snr, data_ser, distance) => {
  document.getElementById("latitude").innerHTML = gps_lat;
  document.getElementById("longitude").innerHTML = gps_lng;
  document.getElementById("altitude").innerHTML = gps_alt;
  document.getElementById("baroAltitude").innerHTML = baro_alt;
  document.getElementById("speed").innerHTML = speed;
  document.getElementById("temperature").innerHTML = temperature;
  document.getElementById("rssi").innerHTML = rssi;
  document.getElementById("snr").innerHTML = snr;
  document.getElementById("distance").innerHTML = distance;
  document.getElementById("rawReceivedData").innerHTML += data_ser + "\r\n";
  document.getElementById("rawReceivedData").scrollTop = document.getElementById("rawReceivedData").scrollHeight;
})

socket.on("update_map", () => {
  if (document.getElementById("mapSwitch").checked)
  {
    document.getElementById("map").contentWindow.location.reload(); 
  }
})

window.onload = function(){
  socket.emit("get_data");

  document.getElementById("pingRequest").addEventListener("click", function() {
    socket.emit("ping_request");
  })
  document.getElementById("dataRequest").addEventListener("click", function() {
    socket.emit("data_request");
  })
  document.getElementById("detachRequest").addEventListener("click", function() {
    socket.emit("detach_request");
  })
  document.getElementById("sendOffRequest").addEventListener("click", function() {
    socket.emit("data_send_off_request");
  })
  document.getElementById("sendOnRequest").addEventListener("click", function() {
    socket.emit("data_send_on_request");
  })
  document.getElementById("buzzerOffRequest").addEventListener("click", function() {
    socket.emit("buzzer_off_request");
  })
  document.getElementById("buzzerOnRequest").addEventListener("click", function() {
    socket.emit("buzzer_on_request");
  })
  document.getElementById("predictionRequest").addEventListener("click", function() {
    socket.emit("prediction_request");
  })
  document.getElementById("anglesRequest").addEventListener("click", function() {
    az = document.getElementById("azimuth")
    elev = document.getElementById("elevation")
    if ((az.value != null || az.value != "") && (elev.value != null || elev.value != ""))
    {
      socket.emit("custom_angles", az.value, elev.value);
      az.value = "";
      elev.value = "";
    }
    
  })
  document.getElementById("coordinatesRequest").addEventListener("click", function() {
    lat = document.getElementById("lat")
    lng = document.getElementById("lng")
    alt = document.getElementById("alt")
    if ((lat.value != null || lat.value != "") && (lng.value != null || lng.value != "") && (alt.value != null || alt.value != ""))
    {
      socket.emit("custom_coordinates", lat.value, lng.value, alt.value);
      lat.value = "";
      lng.value = "";
      alt.value = "";
    }
  })
}