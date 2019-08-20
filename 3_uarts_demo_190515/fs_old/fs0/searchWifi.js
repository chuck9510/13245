
function searchWifi(){
	
	var wifiNum=3;
	var scanSelect=1;
    var scanSSID=1;
    var scanBSSID=1;
    var scanRSSI=1;
    var scanChannel=1;
    var scanEncryp=1;
    var scanAuth=2;
    var scanNetType=1;
	
	
	
	for(var i=0; i<wifiNum; i++)
	{
		var insertText ="<tr> <td><input type=radio name=selectedSSID  value= "  
					+scanSSID+  
					"\t onClick=\"selectedSSIDChange(this.value,' " 
					+scanBSSID+ 
					"',' " 
					+scanNetType+ 
					"', " 
					+scanChannel+  
					",' "  
					+scanEncryp+
					"',' "
					+scanAuth+
					 "')\"></td> ";
	
		insertText=insertText + "<td>" + scanSSID + "</td>";
		insertText=insertText + "<td>" + scanBSSID + "</td>";
		insertText=insertText + "<td>" + scanRSSI + "</td>";
		insertText=insertText + "<td>" + scanChannel + "</td>";
		insertText=insertText + "<td>" + scanEncryp + "</td>";
		insertText=insertText + "<td>" + scanAuth + "</td>";
		insertText=insertText + "<td>" + scanNetType + "</td></tr><br/>";
		
		//var userName=document.forms[0].userName.value;
		var searchWifiTable = document.getElementById('searchWifiTable');
		
		//document.getElementById('searchWifiTable').rows[1].innerHTML = document.getElementById('searchWifiTable').rows[1].innerHTML + insertText;
		//var Rows=table1.rows;//类似数组的Rows 
		var newRow=searchWifiTable.insertRow(searchWifiTable.rows.length);//在末尾插入新的一行 
		var newCell = null;
		
		//for(var j=0; j<8; j++)
		//{
			var newCell1 = newRow.insertCell(0);
			newCell1.innerHTML = "<input type=radio name=selectedSSID  value= "  
					+scanSSID+  
					"&nbsp;onClick=\"selectedSSIDChange(this.value,' " 
					+scanBSSID+ 
					"',' " 
					+scanNetType+ 
					"', " 
					+scanChannel+  
					",' "  
					+scanEncryp+
					"',' "
					+scanAuth+
					 "')\">";
			//alert(newCell1.innerHTML);
			
			newCell = newRow.insertCell(1);
			newCell.innerHTML = scanSSID;
			
			newCell = newRow.insertCell(2);
			newCell.innerHTML = scanBSSID;
			
			newCell = newRow.insertCell(3);
			newCell.innerHTML = scanRSSI;
			
			newCell = newRow.insertCell(4);
			newCell.innerHTML = scanChannel;
			
			newCell = newRow.insertCell(5);
			newCell.innerHTML = scanEncryp;
			
			newCell = newRow.insertCell(6);
			newCell.innerHTML = scanAuth;
			
			newCell = newRow.insertCell(7);
			newCell.innerHTML = scanNetType;
		//}
		//var Cells=newRow.cells;//类似数组的Cells  
		//var newCell=Rows(newRow.rowIndex).insertCell(Cells.length); 
		//newCell.align="center"; 
		//var newCell=Rows(newRow.rowIndex).insertCell(Cells.length); 
		//newCell.align="center"; 

		//alert(document.getElementById('searchWifiTable').rows[0].innerHTML);
		//alert(insertText);
		//var div = document.createElement("div"); 
		//div.id = "searchWifiDiv"; 
		//div.innerHTML = insertText; 
		//document.getElementsByTagName("head")[0].appendChild(new_element);
		//document.body.appendChild(div); 

		
	}
}
