<!-- Copyright (c) 2014-2015 Texas Instruments Incorporated.  All rights reserved. -->

window.onload = function()
{
    document.getElementById('about').onclick = loadAbout;
    document.getElementById('overview').onclick = loadOverview;
    document.getElementById('block').onclick = loadBlock;
    document.getElementById('config1').onclick = loadConfig1;
    document.getElementById('config2').onclick = loadConfig2;
    document.getElementById('misc').onclick = loadMisc;
    document.getElementById('upd').onclick = loadUpd;
}

function loadAbout()
{
    loadPage("about.htm");
    return false;
}

function loadOverview()
{
    loadPage("overview.htm");
    return false;
}

function loadBlock()
{
    loadPage("block.htm");
    return false;
}

function loadS2E()
{
    loadPage("s2e.shtm");
    return false;
}

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

function loadMisc()
{
    loadPage("misc.shtm");
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
  FindAndSelect(document.config.tnmode, tnm);
  FindAndSelect(document.config.tnprot, tnp);
  document.config.telnett.value = tt;
  document.config.telnetlp.value = tlp;
  document.config.telnetrp.value = trp;
  document.config.telnetip1.value =tip1;
  document.config.telnetip2.value =tip2;
  document.config.telnetip3.value =tip3;
  document.config.telnetip4.value =tip4;
  SetIPState();
}

function SetIPState()
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

