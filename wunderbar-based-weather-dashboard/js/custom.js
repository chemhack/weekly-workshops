$(document).foundation();

$(document).ready(function() {

    var AppID = "a667dd67-e373-4500-ad1d-49175bb49b3c";
    var toke = "7oXbotENRTjVoleOopFC6M4w5F8AMhs8" ;
    var Temp_deviceId = "98e24551-cdca-416d-828f-158c591f7302";
    var Luminosity_deviceId = "36b4269d-8c11-4949-8a53-b2dde52e2cd5";

    var relayr = RELAYR.init({
        appId: AppID
    });
    //If you have a token and your device ID, you can start listening to your device without going through the login process


    var temp;
    var humid;
    var lumi;
    var clock;

    //Luminosity
    relayr.devices().getDeviceData({
        token: toke,
        deviceId: Luminosity_deviceId,
        incomingData: function(data) {
            lumi = data.readings[0].value;
            lumi = (lumi / 40.96).toFixed(1);
            $("#lumi").html((lumi).toString());
        }
    });


    //Temperature & Humidity
    relayr.devices().getDeviceData({
        token: toke,
        deviceId: Temp_deviceId,
        incomingData: function(data) {
            temp = data.readings[0].value;
            $("#temp").html((temp).toFixed(1).toString());
            humid = data.readings[1].value;
            $("#humid").html((humid).toFixed(1).toString());


            $("#timetext").html(moment().format('MMMM Do YYYY, <br/> HH:mm:ss '));
        }
    }); 
})