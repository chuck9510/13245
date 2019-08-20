/*//控制分组的打开和关闭
var openDiv =  function ()
				{
					console.log("111");
					var div = document.getElementById("ur_set_id");
				
					if(div.style.display == "block"){//如果分组是展开的
						div.style.display = "none";
						return;
					}else{//如果分组是关闭的
						div.style.display = "block";
						return;
					}
				}*/

IPChange = function ()
{
	alert("If you modify the IP address,please re-searching this device!");
}

SetIPState_ms = function ()
      {
        var tm = document.config.tm.value;
        var disable;
        if(tm == 1)
        {
          document.config.flow.disabled = false;
        }
        else
        {
          document.config.flow.disabled = true;
        }
      }
	  
	  function SetIPState()
      {
        var staticip = document.ip.staticip.value;
        var disable;
        if(staticip == 0)
        {
          disable = true;
        }
        else
        {
          disable = false;
        }
        document.ip.sip1.disabled = disable;
        document.ip.sip2.disabled = disable;
        document.ip.sip3.disabled = disable;
        document.ip.sip4.disabled = disable;
        document.ip.mip1.disabled = disable;
        document.ip.mip2.disabled = disable;
        document.ip.mip3.disabled = disable;
        document.ip.mip4.disabled = disable;
        document.ip.gip1.disabled = disable;
        document.ip.gip2.disabled = disable;
        document.ip.gip3.disabled = disable;
        document.ip.gip4.disabled = disable;
      }
	  
SetIPState_LAN = function ()
{
  var ws = document.config.ws.value;
  var tnp = document.config.tnprot.value;  

  if((ws == 3) && (tnp == 0))//tcp server with Telnet protocol
  {
  document.config.telnetip1.disabled = true;
  document.config.telnetip2.disabled = true;
  document.config.telnetip3.disabled = true;
  document.config.telnetip4.disabled = true;
  document.config.udpip1.disabled = true;
  document.config.udpip2.disabled = true;
  document.config.udpip3.disabled = true;
  document.config.udpip4.disabled = true;
  document.config.telnetrp.disabled = true;
  document.config.telnetlp.disabled = false;
  //document.config.mx.disabled = false; 
  document.config.tnprot.disabled = false;
  document.config.modbus.disabled = true;
  document.config.telnett.disabled = false;  
  }

  if((ws == 3) && (tnp == 2))//tcp server with Raw protocol
  {
  document.config.telnetip1.disabled = true;
  document.config.telnetip2.disabled = true;
  document.config.telnetip3.disabled = true;
  document.config.telnetip4.disabled = true;
  document.config.udpip1.disabled = true;
  document.config.udpip2.disabled = true;
  document.config.udpip3.disabled = true;
  document.config.udpip4.disabled = true;
  document.config.telnetrp.disabled = true;
  document.config.telnetlp.disabled = false;
  //document.config.mx.disabled = false; 
  document.config.tnprot.disabled = false;
  document.config.modbus.disabled = false;
  document.config.telnett.disabled = false;  
  }

  if((ws == 1) && (tnp == 0))//tcp client with Telnet protocol
  {
  document.config.telnetip1.disabled = false;
  document.config.telnetip2.disabled = false;
  document.config.telnetip3.disabled = false;
  document.config.telnetip4.disabled = false;
  document.config.udpip1.disabled = true;
  document.config.udpip2.disabled = true;
  document.config.udpip3.disabled = true;
  document.config.udpip4.disabled = true;
  document.config.telnetrp.disabled = false;
  document.config.telnetlp.disabled = false;
  //document.config.mx.disabled = true; 
  document.config.tnprot.disabled = false;
  document.config.modbus.disabled = true;
  document.config.telnett.disabled = false;  
  }

  if((ws == 1) && (tnp == 2))//tcp client with Raw protocol
  {
  document.config.telnetip1.disabled = false;
  document.config.telnetip2.disabled = false;
  document.config.telnetip3.disabled = false;
  document.config.telnetip4.disabled = false;
  document.config.udpip1.disabled = true;
  document.config.udpip2.disabled = true;
  document.config.udpip3.disabled = true;
  document.config.udpip4.disabled = true;
  document.config.telnetrp.disabled = false;
  document.config.telnetlp.disabled = false;
  //document.config.mx.disabled = true; 
  document.config.tnprot.disabled = false;
  document.config.modbus.disabled = false;
  document.config.telnett.disabled = false;   
  }

  if(ws == 0)//udp
  {
  document.config.telnetip1.disabled = true;
  document.config.telnetip2.disabled = true;
  document.config.telnetip3.disabled = true;
  document.config.telnetip4.disabled = true;
  document.config.udpip1.disabled = false;
  document.config.udpip2.disabled = false;
  document.config.udpip3.disabled = false;
  document.config.udpip4.disabled = false;
  document.config.telnetrp.disabled = false;
  document.config.telnetlp.disabled = false;
  //document.config.mx.disabled = true;
  document.config.tnprot.disabled = true; 
  document.config.modbus.disabled = true;
  document.config.telnett.disabled = true;  
  }
}	  

FindAndSelect = function (selectBox, value)
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

FindNearestAndSelect = function (selectBox, value)
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

 SetFormDefaults_LAN = function ()
 {
	//com0
	console.log("SetFormDefaults_LAN");
  FindNearestAndSelect(document.config.br, br);
  FindAndSelect(document.config.stop, sb);
  FindAndSelect(document.config.bc, bc);
  FindAndSelect(document.config.flow, fc);
  FindAndSelect(document.config.parity, par);
  FindAndSelect(document.config.tm, tm);/*chris*/
 
    FindAndSelect(document.config.ws, ws);/*chris_web*/
  //FindAndSelect(document.config.mx, mx);/*chris_web*/
  //FindAndSelect(document.config.RFCEN, rfc); 
  FindAndSelect(document.config.tnprot, tnp);

  FindAndSelect(document.config.modbus, mb);
  document.config.telnett.value = tt;
    document.config.packtime.value = pt;
    document.config.packlen.value = pl;	
  document.config.telnetlp.value = tlp;
  document.config.telnetrp.value = trp;
  document.config.telnetip1.value =tip1;
  document.config.telnetip2.value =tip2;
  document.config.telnetip3.value =tip3;
  document.config.telnetip4.value =tip4;
  document.config.udpip1.value =uip1;
  document.config.udpip2.value =uip2;
  document.config.udpip3.value =uip3;
  document.config.udpip4.value =uip4;  

    //ip address select
   FindAndSelect(document.ip.staticip, staticip);
        document.ip.sip1.value = sip1;
        document.ip.sip2.value = sip2;
        document.ip.sip3.value = sip3;
        document.ip.sip4.value = sip4;
        document.ip.mip1.value = mip1;
        document.ip.mip2.value = mip2;
        document.ip.mip3.value = mip3;
        document.ip.mip4.value = mip4;
        document.ip.gip1.value = gip1;
        document.ip.gip2.value = gip2;
        document.ip.gip3.value = gip3;
        document.ip.gip4.value = gip4;
	SetIPState_LAN();
	SetIPState_ms();
	SetIPState();
}



