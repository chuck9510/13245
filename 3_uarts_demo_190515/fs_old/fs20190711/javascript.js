function IPChange()
{
	alert("If you modify the IP address,please re-searching this device!");
}

function SetIPState_ms()
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
	  
function SetIPState_LAN()
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

function SetFormDefaults_LAN()
{
	//com0
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
  
  //com1
   FindNearestAndSelect(document.config.br1, br1);
  FindAndSelect(document.config.stop1, sb1);
  FindAndSelect(document.config.bc1, bc1);
  FindAndSelect(document.config.flow1, fc1);
  FindAndSelect(document.config.parity1, par1);
  FindAndSelect(document.config.tm1, tm1);/*chris*/
  
    FindAndSelect(document.config.ws1, ws1);/*chris_web*/
  //FindAndSelect(document.config.mx, mx);/*chris_web*/
  //FindAndSelect(document.config.RFCEN, rfc); 
  FindAndSelect(document.config.tnprot1, tnp1);

  FindAndSelect(document.config.modbus1, mb);
  document.config.telnett1.value = tt1;
    document.config.packtime1.value = pt1;
    document.config.packlen1.value = pl1;	
  document.config.telnetlp1.value = tlp1;
  document.config.telnetrp1.value = trp1;
  document.config.telnetip1_1.value =tip1;
  document.config.telnetip2_1.value =tip2;
  document.config.telnetip3_1.value =tip3;
  document.config.telnetip4_1.value =tip4;
  document.config.udpip1_1.value =uip1;
  document.config.udpip2_1.value =uip2;
  document.config.udpip3_1.value =uip3;
  document.config.udpip4_1.value =uip4;  
  
  //com2
     FindNearestAndSelect(document.config.br2, br2);
  FindAndSelect(document.config.stop2, sb2);
  FindAndSelect(document.config.bc2, bc2);
  FindAndSelect(document.config.flow2, fc2);
  FindAndSelect(document.config.parity2, par2);
  FindAndSelect(document.config.tm2, tm2);/*chris*/
  
     FindAndSelect(document.config.ws2, ws2);/*chris_web*/
  //FindAndSelect(document.config.mx, mx);/*chris_web*/
  //FindAndSelect(document.config.RFCEN, rfc); 
  FindAndSelect(document.config.tnprot2, tnp2);

  FindAndSelect(document.config.modbus2, mb);
  document.config.telnett2.value = tt2;
    document.config.packtime2.value = pt2;
    document.config.packlen2.value = pl2;	
  document.config.telnetlp2.value = tlp2;
  document.config.telnetrp2.value = trp2;
  document.config.telnetip1_2.value =tip1;
  document.config.telnetip2_2.value =tip2;
  document.config.telnetip3_2.value =tip3;
  document.config.telnetip4_2.value =tip4;
  document.config.udpip1_2.value =uip1;
  document.config.udpip2_2.value =uip2;
  document.config.udpip3_2.value =uip3;
  document.config.udpip4_2.value =uip4;  

  
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



