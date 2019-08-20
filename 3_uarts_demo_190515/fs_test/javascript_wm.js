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

//second parameter come from hw
function SetFormDefaults_wm()
{
  FindAndSelect(document.wifimode.wifimd, wm); 
  FindAndSelect(document.wifitm.wifiprol, tprol); 
  document.wifitm.wflp.value = lp; 
  document.wifitm.wfrp.value = rp; 
  document.wifitm.wfip1.value = wfip1; 
  document.wifitm.wfip2.value = wfip2; 
  document.wifitm.wfip3.value = wfip3; 
  document.wifitm.wfip4.value = wfip4;  
  SetIPState_WiFi();
 }

function SetIPState_WiFi()
      {
        var prol = document.wifitm.wifiprol.value;
		if(prol == 3)//TCP Server
        {
		document.wifitm.wflp.disabled = false;
		document.wifitm.wfrp.disabled = true;
        document.wifitm.wfip1.disabled = true;
        document.wifitm.wfip2.disabled = true;
        document.wifitm.wfip3.disabled = true;
        document.wifitm.wfip4.disabled = true;
		}
		if((prol == 1) || (prol == 0))//TCP Client / UDP
		{
		document.wifitm.wflp.disabled = false;
		document.wifitm.wfrp.disabled = false;
		document.wifitm.wfip1.disabled = false;
		document.wifitm.wfip2.disabled = false;
		document.wifitm.wfip3.disabled = false;
		document.wifitm.wfip4.disabled = false;
		}
/*		if(udp_cue ==1)//TCP Client / UDP
		{
		alert("Please click the button for reboot to make the UDP setting effect!");
		}
		*/
      }