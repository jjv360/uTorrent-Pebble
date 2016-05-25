//
// Pebble JS app

/**
*
*  Base64 encode / decode
*  http://www.webtoolkit.info/
*
**/
var Base64 = {

	// private property
	_keyStr : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",

	// public method for encoding
	encode : function (input) {
		var output = "";
		var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
		var i = 0;

		input = Base64._utf8_encode(input);

		while (i < input.length) {

			chr1 = input.charCodeAt(i++);
			chr2 = input.charCodeAt(i++);
			chr3 = input.charCodeAt(i++);

			enc1 = chr1 >> 2;
			enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
			enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
			enc4 = chr3 & 63;

			if (isNaN(chr2)) {
				enc3 = enc4 = 64;
			} else if (isNaN(chr3)) {
				enc4 = 64;
			}

			output = output +
			this._keyStr.charAt(enc1) + this._keyStr.charAt(enc2) +
			this._keyStr.charAt(enc3) + this._keyStr.charAt(enc4);

		}

		return output;
	},

	// public method for decoding
	decode : function (input) {
		var output = "";
		var chr1, chr2, chr3;
		var enc1, enc2, enc3, enc4;
		var i = 0;

		input = input.replace(/[^A-Za-z0-9\+\/\=]/g, "");

		while (i < input.length) {

			enc1 = this._keyStr.indexOf(input.charAt(i++));
			enc2 = this._keyStr.indexOf(input.charAt(i++));
			enc3 = this._keyStr.indexOf(input.charAt(i++));
			enc4 = this._keyStr.indexOf(input.charAt(i++));

			chr1 = (enc1 << 2) | (enc2 >> 4);
			chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
			chr3 = ((enc3 & 3) << 6) | enc4;

			output = output + String.fromCharCode(chr1);

			if (enc3 != 64) {
				output = output + String.fromCharCode(chr2);
			}
			if (enc4 != 64) {
				output = output + String.fromCharCode(chr3);
			}

		}

		output = Base64._utf8_decode(output);

		return output;

	},

	// private method for UTF-8 encoding
	_utf8_encode : function (string) {
		string = string.replace(/\r\n/g,"\n");
		var utftext = "";

		for (var n = 0; n < string.length; n++) {

			var c = string.charCodeAt(n);

			if (c < 128) {
				utftext += String.fromCharCode(c);
			}
			else if((c > 127) && (c < 2048)) {
				utftext += String.fromCharCode((c >> 6) | 192);
				utftext += String.fromCharCode((c & 63) | 128);
			}
			else {
				utftext += String.fromCharCode((c >> 12) | 224);
				utftext += String.fromCharCode(((c >> 6) & 63) | 128);
				utftext += String.fromCharCode((c & 63) | 128);
			}

		}

		return utftext;
	},

	// private method for UTF-8 decoding
	_utf8_decode : function (utftext) {
		var string = "";
		var i = 0;
		var c = 0;
		//var c1 = 0;
		var c2 = 0;
		var c3 = 0;

		while ( i < utftext.length ) {

			c = utftext.charCodeAt(i);

			if (c < 128) {
				string += String.fromCharCode(c);
				i++;
			}
			else if((c > 191) && (c < 224)) {
				c2 = utftext.charCodeAt(i+1);
				string += String.fromCharCode(((c & 31) << 6) | (c2 & 63));
				i += 2;
			}
			else {
				c2 = utftext.charCodeAt(i+1);
				c3 = utftext.charCodeAt(i+2);
				string += String.fromCharCode(((c & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
				i += 3;
			}

		}

		return string;
	}

};

// Vars
var token = null;
var cacheId = null;


// Start app
Pebble.addEventListener("ready", function() {
	
	
	
});

// Show configuration page
Pebble.addEventListener("showConfiguration", function(e) {
	
	// Create config
	var config = {
		host: localStorage.host,
		user: localStorage.user,
		pass: localStorage.pass
	};
	
	// Launch settings page
	Pebble.openURL("http://www.ydangleapps.com/uTorrentRemoteSettings/index.html#" + encodeURIComponent(JSON.stringify(config)));
	
});

// Return from settings screen
Pebble.addEventListener("webviewclosed", function(e) {
    
	// Save config info
	if (!e.response) return;
	var config = JSON.parse(decodeURIComponent(e.response));
    localStorage.host = config.host;
    localStorage.user = config.user;
    localStorage.pass = config.pass;
	
	// Do another check
	checkTorrents();
	
});

// Receive app message
Pebble.addEventListener("appmessage", function(e) {
	
	// Check message type
	var msg = e.payload;
	if (msg.action == "update") {
		
		// Start checking for torrents
		checkTorrents();
		
	} else if (msg.action == "start") {
		
		// Send request
		doRequest("GET", "/gui/?action=start&token=" + token + "&hash=" + msg.hash , null, function(data) {
			
			// Started successfully
			sendMsg({"action": "set_active", "hash": msg.hash, "value": false});
			
			// Update info
			setTimeout(checkTorrents, 500);
			
		}, function(err) {
			
			// Failed
			console.log("Failed to start torrent " + msg.hash);
			
		});
		
	} else if (msg.action == "stop") {
		
		// Send request
		doRequest("GET", "/gui/?action=stop&token=" + token + "&hash=" + msg.hash , null, function(data) {
			
			// Stopped successfully
			sendMsg({"action": "set_active", "hash": msg.hash, "value": false});
			
			// Update info
			checkTorrents();
			
		}, function(err) {
			
			// Failed
			console.log("Failed to stop torrent " + msg.hash);
			
		});
		
	}
	
});

var msgs = [];
var busySending = false;
function sendMsg(msg) {
	
	// Add message to queue
	msgs.push(msg);
			
	// Start sending if not busy sending
	if (!busySending)
		sendNextMsg();
	
}
			
function sendNextMsg() {
	
	// Check if there are any more messages
	busySending = true;
	if (msgs.length === 0) {
		busySending = false;
		return;
	}
			
	// Get next message
	var msg = msgs[0];
	msgs.splice(0, 1);

	// Send message
	Pebble.sendAppMessage(msg, function() {
	
		// Passed
		sendNextMsg();
	
	}, function() {
	
		// Failed
		console.log("Failed to send message: " + JSON.stringify(msg));
		sendNextMsg();
	
	});
	
}

function doRequest(method, url, data, successCallback, failCallback) {
	
	var req = new XMLHttpRequest();
	var fullurl = "http://" + localStorage.host + url;
	req.open(method, fullurl, true);
	req.setRequestHeader("Authorization", "Basic " + Base64.encode(localStorage.user + ":" + localStorage.pass));
	req.send(data);
	req.onreadystatechange = function() {
		
		if (req.readyState == 4 && req.status == 200)
			successCallback(req.responseText);
		else if (req.readyState == 4)
			failCallback(req.status);
		
	};
	
	console.log("Sending request to: " + fullurl);
	
}

var launchedSettings = false;
function checkTorrents() {
	
	// Check for valid host name
	if (!localStorage.host) {
		
		// Launch settings page once
		if (!launchedSettings)
			Pebble.openURL("http://www.ydangleapps.com/uTorrentRemoteSettings.html");
		
		// Send invalid settings message to app
		launchedSettings = true;
		sendMsg({"action": "invalid_settings"});
		return;
		
	}
	
	// Begin check
	sendMsg({"action": "update_started"});
	
	// Get token if needed
	if (!token) {
		getToken();
		return;
	}
	
	// Started getting list of torrents from server
	sendMsg({"action": "listing_torrents"});
	
	// Do request
	var url = "/gui/?list=1&token=" + token;
	if (cacheId) url += "&cid=" + cacheId;
	doRequest("GET", url, null, function(data) {
		
		// Got torrent list
		var json = JSON.parse(data);
		if (json.torrents)
		for (var i = 0 ; i < json.torrents.length ; i++) {
			
			// Create torrent
			var t = json.torrents[i];
			var torrent = {
				hash: t[0],
				active: (t[1] & 1) || (t[1] & 2),
				name: t[2],
				size: t[3],
				downloaded: t[5],
				uploaded: t[6],
				uploadSpeed: t[8],
				downloadSpeed: t[9],
				timeLeft: t[10]
			};
			
			// Send to app
			sendTorrentState(torrent);
			
		}
		
		// Got updated torrent list
		if (json.torrentp)
		for (var x = 0 ; x < json.torrentp.length ; x++) {
			
			// Create torrent
			var tp = json.torrentp[x];
			var torrentp = {
				hash: tp[0],
				active: (tp[1] & 1) || (tp[1] & 2),
				name: tp[2],
				size: tp[3],
				downloaded: tp[5],
				uploaded: tp[6],
				uploadSpeed: tp[8],
				downloadSpeed: tp[9],
				timeLeft: tp[10]
			};
			
			// Send to app
			sendTorrentState(torrentp);
			
		}
		
		// Got removed torrent list
		if (json.torrentm)
		for (var y = 0 ; y < json.torrentm.length ; y++) {
			
			// Send to app
			sendMsg({"action": "remove_torrent", "hash": json.torrentm[y]});
			
		}
		
		// Done
		sendMsg({"action": "update_finished"});
		
		// Save cache id
		cacheId = json.torrentc;
		
	}, function(err) {
		
		// Failed
		console.log("Failed");
		token = null;
		cacheId = null;
		sendMsg({"action": "connection_failed"});
		
	});
	
}

function sendTorrentState(torrent) {
	
	// Trim name
	if (torrent.name.length > 40)
		torrent.name = torrent.name.substring(0, 40);
	
	// Send values
	sendMsg({"action": "set_name", "hash": torrent.hash, "value": torrent.name});
	sendMsg({"action": "set_active", "hash": torrent.hash, "value": torrent.active});
	sendMsg({"action": "set_size", "hash": torrent.hash, "value": torrent.size/1024});
	sendMsg({"action": "set_downloaded", "hash": torrent.hash, "value": torrent.downloaded/1024});
	sendMsg({"action": "set_uploaded", "hash": torrent.hash, "value": torrent.uploaded/1024});
	sendMsg({"action": "set_downloadSpeed", "hash": torrent.hash, "value": torrent.downloadSpeed});
	sendMsg({"action": "set_uploadSpeed", "hash": torrent.hash, "value": torrent.uploadSpeed});
	sendMsg({"action": "set_timeLeft", "hash": torrent.hash, "value": torrent.timeLeft});
	
}

function getToken() {
	
	// Do request
	cacheId = null;
	doRequest("GET", "/gui/token.html", null, function(data) {
		
		// Success, find token
		var firstIndex = data.indexOf(";'>");
		var secondIndex = data.indexOf("</div");
		if (firstIndex == -1 || secondIndex == -1) {
			sendMsg({"action": "invalid_login"});
			return;
		}
		
		token = data.substring(firstIndex+3, secondIndex);
		console.log("Got token: " + token);
		checkTorrents();
		
	}, function(code) {
		
		// Error
		if (code == 401)
			sendMsg({"action": "invalid_login"});
		else
			sendMsg({"action": "connection_failed"});
		
		console.log("Error: " + code);
		
	});
	
}