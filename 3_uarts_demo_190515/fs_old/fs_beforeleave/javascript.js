<!-- Copyright (c) 2014-2015 Texas Instruments Incorporated.  All rights reserved. -->

window.onload = function()
{
    document.getElementById('qc').onclick = loadqc;/*seabird0808*/
    document.getElementById('ms').onclick = loadms;/*seabird0808*/
	document.getElementById('wlanap').onclick = loadwlanap;
	document.getElementById('wlansta').onclick = loadwlansta;
	document.getElementById('lan').onclick = loadlan;
	document.getElementById('wlan2ur').onclick = loadwlan2ur;/*end*/
    document.getElementById('s2e').onclick = loadS2E;/*chris_web*/
	document.getElementById('dev').onclick = loaddev;
	document.getElementById('udp').onclick = loadUpd;/*chris_web*/
	
}

function loadqc()
{
    loadPage("qc.shtm");
    return false;
}

function loadms()
{
    loadPage("ms.shtm");
    return false;
}

function loadlan()
{
    loadPage("lan.shtm");
    return false;
}

function loadS2E()
{
    loadPage("s2e.shtm");
    return false;
}
/*seabird0808-
function loadConfig1()
{
    loadPage("config1.shtm");
    return false;
}

function loadConfig2()
{
    loadPage("config2.shtm");
    return false;
}
*/
/*****/
//chris_web
function CheckDefaults(myForm)
      {
        var answer = confirm("This will erase all existing configuration changes and restore factory default settings. Click OK if you are sure you want to do this or Cancel to retain existing settings.");
        if(answer == false)
        {
          alert("Existing configuration settings have been retained.");
        }
        return(answer);
      }
/*add wireless lan config page,seabird0805*/
function loadwlan()
{
    loadPage("wlan.shtm");
    return false;
}
function loadwlanap()
{
    loadPage("wlanap.shtm");
    return false;
}
function loadwlansta()
{
    loadPage("wlansta.shtm");
    return false;
}
function loadwlan2ur()
{
    loadPage("wlan2ur.shtm");
    return false;
}
/*end*/
/*seabird0808+*/
function loaddev()
{
    loadPage("dev.shtm");
    return false;
}

function loadUpd()
{
    loadPage("upd.shtm");
    return false;
}

function loadPage(page)
{
    if(window.XMLHttpRequest)
    {
        xmlhttp = new XMLHttpRequest();
    }
    else
    {
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
    }

    xmlhttp.open("GET", page, true);
    xmlhttp.setRequestHeader("Content-type",
                             "application/x-www-form-urlencoded");
    xmlhttp.send();

    xmlhttp.onreadystatechange = function ()
    {
        if((xmlhttp.readyState == 4) && (xmlhttp.status == 200))
        {
            document.getElementById("content").innerHTML = xmlhttp.responseText;
        }
    }
}

function FindAndSelect(selectBox, value)
{
  var i = 0;
  for(i = 0; i < selectBox.length; i++)
  {
    if(selectBox.options[i].value == value)
    {
      selectBox.selectedIndex = i;
      return;
    }
  }
}

function FindNearestAndSelect(selectBox, value)
{
  var i = 0;
  var min;
  var max;

  for(i = 0; i < selectBox.length; i++)
  {
    min = (value * 90) / 100;
    max = (value * 110) / 100;

    if((selectBox.options[i].value > min) && (selectBox.options[i].value < max))
    {
      selectBox.selectedIndex = i;
      return;
    }
  }
}

function SetFormDefaults()
{
  FindNearestAndSelect(document.config.br, br);
  FindAndSelect(document.config.stop, sb);
  FindAndSelect(document.config.bc, bc);
  FindAndSelect(document.config.flow, fc);
  FindAndSelect(document.config.parity, par);
  FindAndSelect(document.config.tnmode, tnm);//?
  FindAndSelect(document.config.tnprot, tnp);//?
  FindAndSelect(document.config.wlanmode, wm);/*seabird0805*/
  FindAndSelect(document.config.tm, tm);/*chris*/
  FindAndSelect(document.config.ws, ws);/*chris_web*/
  FindAndSelect(document.config.mx, mx);/*chris_web*/
  document.config.telnett.value = tt;
  document.config.telnetlp.value = tlp;
  document.config.telnetrp.value = trp;
  document.config.telnetip1.value =tip1;
  document.config.telnetip2.value =tip2;
  document.config.telnetip3.value =tip3;
  document.config.telnetip4.value =tip4;
  SetIPState();
}

function SetIPState()//?
{
  var tnmode = document.config.tnmode.value;
  var disable;
  if(tnmode == 0)
  {
    disable = true;
  }
  else
  {
    disable = false;
  }
  document.config.telnetip1.disabled = disable;
  document.config.telnetip2.disabled = disable;
  document.config.telnetip3.disabled = disable;
  document.config.telnetip4.disabled = disable;
  document.config.telnetrp.disabled = disable;
}

