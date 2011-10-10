function doOnloadStuff() {
	
}

function toggleVisibility( elem ) {
	if( ! elem ) return;
	elem.style.display = (elem.style.display == "") ? "none" : "";
}

function handleKeyDown( e ) {

	if( e == null && event != null ) {
		e = event;
	}

	if( e == null ) {
		return true;
	}
	
	var keycode = (e.which == null) ? e.keyCode : e.which;

	var id;
	
	switch( keycode ) {

		case 39:
		case 177: // Wii Right
			id = "previous";
		break;

		case 37:
		case 178: // Wii Left
			id = "next";
		break;

		case 38:
		case 175: // Wii Up
			id = "index";
		break;

		case 40:
		case 176: // Wii Down
			id = "comments";
		break;

		case 61:
		case 107:
		case 174: // Wii +
			id = "good";
		break;

		case 109:
		case 170: // Wii -
			id = "bad";
		break;

		case 80: // p
			id = "pickUp";
		break;

		
	}
	
	if( id && document.getElementById( id ) ) {
		document.location.href = document.getElementById( id ).href;
		return false;
	}

	return true;

}

