function ReplaceID(itemID, what)
{
    let it = document.getElementById(itemID);
    it.innerHTML = what;
}
function Get(theUrl, callback, funcfail, funcgood)
{
    let xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() { 
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200) {
            funcgood();
            callback(xmlHttp.responseText);
        }
    }
    xmlHttp.onerror = function(e) {
        funcfail();
        setTimeout(Get, 1000, theUrl, callback, funcfail, funcgood);
    };
    xmlHttp.open("GET", theUrl, true); // true for asynchronous 
    xmlHttp.send(null);
}

function autoUpdate(timeout, disconnectid, temppath, tempnam, humpath, humnam, prevpath, prevnam)
{
    let handleerr = function() {
        let it = document.getElementById(disconnectid);
        it.style.display = 'block';
    };
    let handlegood = function() {
        let it = document.getElementById(disconnectid);
        it.style.display = 'none';
    };
    
    Get(temppath, function(data){
        ReplaceID(tempnam, data);
        Get(humpath, function(data){
            ReplaceID(humnam, data);
            Get(prevpath, function(data){
                ReplaceID(prevnam, data);
                setTimeout(autoUpdate, timeout, timeout, disconnectid, temppath, tempnam, humpath, humnam, prevpath, prevnam);
            }, handleerr, handlegood);
        }, handleerr, handlegood);
    }, handleerr, handlegood);    
}