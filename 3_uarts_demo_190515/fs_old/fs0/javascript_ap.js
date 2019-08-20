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
function SetFormDefaults_ap()
{
  FindAndSelect(document.wifiap.wififre, apf);
  FindAndSelect(document.wifiap.wifidt, apdh);
   
  //IP
  document.wifiap.wip1.value =apip1;
  document.wifiap.wip2.value =apip2;
  document.wifiap.wip3.value =apip3;
  document.wifiap.wip4.value =apip4;
  
  //MASK
  document.wifiap.wmip1.value =apmk1;
  document.wifiap.wmip2.value =apmk2;
  document.wifiap.wmip3.value =apmk3;
  document.wifiap.wmip4.value =apmk4;
  
  //Gateway
  document.wifiap.gtip1.value =apgip1;
  document.wifiap.gtip2.value =apgip2;
  document.wifiap.gtip3.value =apgip3;
  document.wifiap.gtip4.value =apgip4;
  
  document.wifiap.wifinn.value =apsid;
  document.wifiap.appw.value =apw;
 }