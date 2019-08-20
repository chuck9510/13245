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

function SetIPState()
      {
        var staticip = document.wifista.wifict.value;
        var disable;
        if(staticip == 1)
        {
          disable = true;
        }
        else
        {
          disable = false;
        }
        document.wifista.ip1.disabled = disable;
        document.wifista.ip2.disabled = disable;
        document.wifista.ip3.disabled = disable;
        document.wifista.ip4.disabled = disable;
        document.wifista.mkip1.disabled = disable;
        document.wifista.mkip2.disabled = disable;
        document.wifista.mkip3.disabled = disable;
        document.wifista.mkip4.disabled = disable;
        document.wifista.gwip1.disabled = disable;
        document.wifista.gwip2.disabled = disable;
        document.wifista.gwip3.disabled = disable;
        document.wifista.gwip4.disabled = disable;
      }

//second parameter come from hw
function SetFormDefaults_sta()
{
  FindAndSelect(document.wifista.wifict, stacon);
  
  document.wifista.ip1.value =stcip1;
  document.wifista.ip2.value =stcip2;
  document.wifista.ip3.value =stcip3;
  document.wifista.ip4.value =stcip4;
  
  document.wifista.mkip1.value =stcms1;
  document.wifista.mkip2.value =stcms2;
  document.wifista.mkip3.value =stcms3;
  document.wifista.mkip4.value =stcms4;
  
  document.wifista.gwip1.value =stcgw1;
  document.wifista.gwip2.value =stcgw2;
  document.wifista.gwip3.value =stcgw3;
  document.wifista.gwip4.value =stcgw4;
  
  document.wifista.wifipw.value =stapw;
  document.wifista.wifiapid.value =apssid;
  
  SetIPState();
 }