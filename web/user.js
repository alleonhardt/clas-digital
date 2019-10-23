class User {
	//Static because it does not matter which user asks to end his session
	static DoLogout() {
		//Just tell the server to log the current user out, then reload the page
		 var now = new Date();
  		var time = now.getTime();
  		var expireTime = time - 10;
  		now.setTime(expireTime);
  		document.cookie = 'SESSID=;expires='+now.toGMTString()+';';
		window.location = window.location;
	}

	static initialise() {
		let inp = document.getElementById("SpecialSID");
		inp.focus();
		inp.addEventListener("keypress",function(event){
			if(event.key == "Enter")
			{
				document.getElementById("srchButton").click();
			}
		});
		//inp.addEventListener("input",function(event){
		//	document.getElementById("srchButton").click();
		//});

		let vars = document.cookie;
		console.log("Document cookie: "+ document.cookie);
		let pos = vars.indexOf("SESSID=");
		if(pos!=-1)
		{
			let numpos = vars.indexOf(";",pos+7);
			let cookie = "";
			if(numpos==-1)
				cookie = vars.substr(pos+7);
			else
				cookie = vars.substr(pos+7,numpos);

			console.log(cookie);
			let pos2 = cookie.indexOf("::");
			ServerDataObj.user = {};
			ServerDataObj.user.email = cookie.substr(0,pos2);
			ServerDataObj.user.access = parseInt(cookie.substr(pos2+2,cookie.indexOf(":::")));
		}

		if(ServerDataObj==undefined)
			alert("Site internal error");
		else if(ServerDataObj.user == undefined)
		{
				let x = document.getElementById("loggedintopnav");
				x.innerHTML = '<form id ="LoginForm" class="form-inline my-2 my-lg-0" action="/login" method="post"><input id="EmailType" class="form-control mr-sm-2" type="email" name="email" placeholder="Email" aria-label="Email" required><input id="PasswordType" class="from-control mr-sm-2" type="password" name="password" placeholder="Password" aria-label="Password" required><input class="btn btn-outline-success my-2 my-sm-0" type="submit" value="Log In"/></form>';
		}
		else
		{
			document.getElementById("LoggedInAs").innerHTML = "Logged in as <strong>"+ServerDataObj.user.email+"</strong>";

			let list = document.getElementsByClassName("classifiedContent");
			//Unlocks all the content that the user can access!
			for(var i=0; i < list.length; i++)
			{
				let y = list[i].dataset.acc;
				if((ServerDataObj.user.access&y)!=0)
				{
					list[i].style.display="block";
				}
			}
		}
	}
}
