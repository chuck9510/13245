function getCookie(c_name)
 
{
 
	if(document.cookie.length>0)
 
	{
 
		c_start=document.cookie.indexOf(c_name+"=");
 
		if(c_start!=-1)
 
		{
 
 
			c_start=c_start+c_name.length+1;
 
			c_end=document.cookie.indexOf(";",c_start);
 
			if(c_end==-1)
			c_end=document.cookie.length;
 
 
			return unescape(document.cookie.substring(c_start,c_end));
 
		}
 
	}
 
	return "";
 
}
 
 
function setCookie(c_name,value)
 
{
 
	//document.cookie=c_name+"="+escape(value)+";";
	//document.cookie="username=1;password=-1;";
	document.cookie="username=admin";
	document.cookie="path=/";
	//document.cookie="expires=-1";
	//alert("document.cookie="+document.cookie);
	 
}
 
 
function checkCookie()
 
{
 
	var username=null;
	username = getCookie('username');
	//alert("username="+username);
 
	if(username==null || username=="")
	{
		window.location.href="login.shtm";
		return;
	}
 
}