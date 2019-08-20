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
  //for channel switch
  FindAndSelect(document.wifimode.wifimd, wm); 
 }

