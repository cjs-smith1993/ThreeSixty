#!/usr/bin/perl

use CGI qw/:standard/;

print header;

print start_html("CGI Test Page"),
	h1("CGI Test Page"),
	h2("Post Test:"),
	start_form(-method=>"POST",
# To redirect this page to another page, uncomment the following line and update the location
		-action=>"test9.pl"
	),
	"Post Value: ",textfield('postKey'),
	submit("Submit Post"),
	end_form;
	if (param('postKey'))
	{
		print "Post Value: ".param('postKey');
	}
	print hr,
	h2("Get Test:"),
	start_form(-method=>"GET",
# To redirect this page to another page, uncomment the following line and update the location
		-action=>"test9.pl"
	),
	"Get Value: ",textfield('getKey'),
	submit("Submit Get"),
	end_form;
	if (param('getKey'))
	{
		print "Get Value: ".param('getKey');
	}

print end_html;

