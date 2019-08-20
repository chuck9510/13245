// JavaScript Document
//add by chuck
	function com0_visible()
	{
		var com0 = document.getElementById("com0");
		var com1 = document.getElementById("com1");
		var com2 = document.getElementById("com2");
		var bt0 = document.getElementById("bt0");
		var bt1 = document.getElementById("bt1");
		var bt2 = document.getElementById("bt2");
		
		com0.style.display="";
		com1.style.display="none";
		com2.style.display="none";
		
		bt0.style.border="2px solid #0000CC";
		bt1.style.border="";
		bt2.style.border="";
	}
	
	function com1_visible()
	{
		var com0 = document.getElementById("com0");
		var com1 = document.getElementById("com1");
		var com2 = document.getElementById("com2");
		var bt0 = document.getElementById("bt0");
		var bt1 = document.getElementById("bt1");
		var bt2 = document.getElementById("bt2");
		//alert(com0);alert(com2);
		com0.style.display="none";
		com1.style.display="";
		com2.style.display="none";
		
		bt0.style.border="";
		bt1.style.border="2px solid #0000CC";
		bt2.style.border="";
	}
	
	function com2_visible()
	{
		var com0 = document.getElementById("com0");
		var com1 = document.getElementById("com1");
		var com2 = document.getElementById("com2");
		var bt0 = document.getElementById("bt0");
		var bt1 = document.getElementById("bt1");
		var bt2 = document.getElementById("bt2");
		//alert(com0);alert(com1);
		com0.style.display="none";
		com1.style.display="none";
		com2.style.display="";
		
		bt0.style.border="";
		bt1.style.border="";
		bt2.style.border="2px solid #0000CC";
	}