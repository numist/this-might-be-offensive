var template;
var old_key;
$(document).ready(function() {
	// set the template var to the template from the HTML
	template = $("#template").val();

	// change the keycode values into readable values
	$(".keycode").each(function(e) {
		code = parseInt($(this).val());
		value = key_value(code);
		$(this).val(value);
	});

	// start click events
	assign_click_events();

	// event for the Add link
	$('#key_add a').click(function(e) {
		handle_row_add($(this),e);
		return false;
	});

	// handler for the submit button
	$('#keynav_form').submit(function(e) {
		handle_save($(this),e);
	});

});

// delete a row from the form
function handle_row_delete(o,e) {
	o.parent().remove();
	check_delete_link();
}

// add a row to the form. re-initialise click events
function handle_row_add(o,e) {
	$('#keynav table').append(template);
	check_delete_link();
	assign_click_events();
}

function assign_click_events() {
	// click event for deleting a row
	$('.key_delete').unbind();
	$('.key_delete').click(function(e) {
		handle_row_delete($(this),e);	
		return false;
	});

	// event for getting focus (recording a key)
	$('.keycode').unbind();
	$('.keycode').focus(function(e) {
		record_key($(this),e);
	});

	// event for changing the dropdown box back to default
	$('.keytype').unbind();
	$('.keytype select').change(function(e) {
		if($(this).val() == 'default') {
			$(this).parent().parent().find('.keycode').val('');
		}
	});
}

function check_delete_link() {
	// hide the delete if only 1 row left
	count_rows = $('.key_delete').length;
	if(count_rows == 1) {
		$('.key_delete a').css('display', 'none');
		$('.key_delete').unbind();
	} else {
		$('.key_delete a:first').css('display', 'block');
	}
}

function clear_recording(o,e) {
	$('.key_notification').each(function(i) {
		if($(this).html() == "recording") {
			$(this).parent().parent().find('.keycode').val(old_key);
			$(this).html('');
		}
	});
}

function record_key(o,e) {
	clear_recording(o,e);
	o.parent().parent().find('.key_notification').html('recording');

	// save the previous value 
	old_key = o.val();

	// wipe any existing value
	o.val('');

	// start a keypress listening event
	o.keydown(function(i) {
		var keycode = (i.which == null) ? i.keyCode : i.which;
		var keyval = keycode;

		i.preventDefault();

		// modifier keys
		if(keycode == 16 || keycode == 17 || keycode == 18 ||
		   keycode == 224 || keycode == 224 ||
		   keycode == 91 || keycode == 92 || keycode == 93) {
			return;
		}
		keyval = key_value(keycode);
		o.blur();
		o.parent().parent().find('.key_notification').html('');
		o.parent().parent().find('.keycode').val(keyval);
		o.parent().parent().find('.registered_key').val(keycode);

		$('.keycode').unbind('keydown');
	});
}

// return human readable key values
function key_value(keycode) {
	// digits
	if(keycode >= 48 && keycode <= 57) {
		return keycode-48+" (key)";
	}
	
	// numpad digits
	if(keycode >= 96 && keycode <= 105) {
		return keycode-96+" (num)";
	}

	if(keycode >= 65 && keycode <= 90) {
		return String.fromCharCode(keycode).toLowerCase();
	}
	switch(keycode) {
		case 27:	return "Esc";
		case 37:	return "←";
		case 38:	return "↑";
		case 39:	return "→";
		case 40:	return "↓";
		case 61:  return "+";
		case 106: return "* (num)";
		case 107: return "+ (num)";
		case 109: return "- (num)";
		case 110: return ". (num)";
		case 111: return "/ (num)";
		case 112:	return "F1";
		case 113:	return "F2";
		case 114:	return "F3";
		case 115:	return "F4";
		case 116:	return "F5";
		case 117:	return "F6";
		case 118:	return "F7";
		case 119:	return "F8";
		case 120:	return "F9";
		case 121:	return "F10";
		case 122:	return "F11";
		case 123:	return "F12";
		case 170: return "- (Wii)";
		case 174: return "+ (Wii)";
		case 175: return "↑ (Wii)";
		case 176: return "↓ (Wii)";
		case 177: return "→ (Wii)";
		case 178: return "← (Wii)";
		case 187: return "=";
		case 189: return "-";

		default:	return keycode;
	}
}

function handle_save(o,e) {
	// before we submit we set an ID and name for all form elements
	var i = 0;
	$('#keynav form table tr').each(function(e) {
		$(this).find('select').attr('name', "key" + i);
		$(this).find('.keycode').attr('name', "ascii" + i);
		$(this).find('.registered_key').attr('name' , "keycode" + i);
		i++;
	});

	// the code falls through to the normal form submit.....
}
