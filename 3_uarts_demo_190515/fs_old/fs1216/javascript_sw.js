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
function SetFormDefaults_sw()
{
  //for channel switch
  FindAndSelect(document.chswitch.chsw, sw);
 }

