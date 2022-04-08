<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
	<meta http-equiv="content-type" content="text/html; charset=utf-8"/>
	<title></title>
	<meta name="generator" content="LibreOffice 7.2.5.2.0 (Linux)"/>
	<meta name="created" content="2022-04-08T14:55:38.653619897"/>
	<meta name="changed" content="2022-04-08T15:31:21.241877224"/>
	<style type="text/css">
		@page { size: 21cm 29.7cm; margin: 2cm }
		p { line-height: 115%; margin-bottom: 0.25cm; background: transparent }
		h2 { margin-top: 0.35cm; margin-bottom: 0.21cm; background: transparent; page-break-after: avoid }
		h2.western { font-family: "Liberation Sans", sans-serif; font-size: 16pt; font-weight: bold }
		h2.cjk { font-family: "Noto Sans CJK SC"; font-size: 16pt; font-weight: bold }
		h2.ctl { font-family: "Droid Sans Devanagari"; font-size: 16pt; font-weight: bold }
		h1 { margin-bottom: 0.21cm; background: transparent; page-break-after: avoid }
		h1.western { font-family: "Liberation Sans", sans-serif; font-size: 18pt; font-weight: bold }
		h1.cjk { font-family: "Noto Sans CJK SC"; font-size: 18pt; font-weight: bold }
		h1.ctl { font-family: "Droid Sans Devanagari"; font-size: 18pt; font-weight: bold }
		a:link { color: #000080; so-language: zxx; text-decoration: underline }
		a:visited { color: #800000; so-language: zxx; text-decoration: underline }
	</style>
</head>
<body lang="de-DE" link="#000080" vlink="#800000" dir="ltr"><p align="center" style="line-height: 100%; margin-top: 0.42cm; margin-bottom: 0.21cm; page-break-after: avoid">
<font face="Liberation Sans, sans-serif"><font size="6" style="font-size: 28pt"><b>Fork
of MultiGeiger</b></font></font></p>
<h1 class="western">Adoptions in this fork:</h1>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">this
fork is based on the master branch of the original MultiGeiger
software (https://github.com/ecocurious2/MultiGeiger.git) as of
08.04.2022</span></font></font></font></p>
<p style="line-height: 0.5cm; margin-bottom: 0cm"><br/>

</p>
<h2 class="western">Motivation:</h2>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">Having
a Air Quality sensor from the Luftdaten project (sensor.community)
installed, I know about problems with dropping Wifi connections,
memory leaks,</span></font></font></font></p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">interrupted
data transfer to the internet portals, etc.</span></font></font></font></p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">And
I also know, without being able to monitor my sensor outside at its
final installation position, I would have been lost.</span></font></font></font></p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">E.g.
to search a good installation position outside, I use a power bank
for the sensor and my cellphone to display the 'Actual Values' page,</span></font></font></font></p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">and
I directly see the Wifi quality. Then you install it there in a
provisional manner and watch the values from inside via Wifi for a
couple of days.</span></font></font></font></p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">If
sending to the portals fails, you can see the http return code on the
log page.</span></font></font></font></p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">You
can actually monitor all values which are normally sent to the serial
port via the log info page via WiFi now,</span></font></font></font></p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">no
need to connect a laptop anymore via USB (which would imply a
restart, resetting the sensor, which could solve/hide an actual
problem ) ...</span></font></font></font></p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">I
have a influx-db running here on a raspberry, where I log all my
weather data locally. Of course I wanted to include this sensor as
well.</span></font></font></font></p>
<p style="line-height: 0.5cm; margin-bottom: 0cm"><br/>

</p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">This
fork bascically ports some functionality from the 'Luftdaten-Sensor'
(sensor.community) to the MultiGeiger,</span></font></font></font></p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">with
some simplifications and enhancements in the code ...</span></font></font></font></p>
<p style="line-height: 0.5cm; margin-bottom: 0cm"><br/>

</p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">It
does NOT touch the original code concerning gathering the data from
the tube, etc. This is all untouched !</span></font></font></font></p>
<p style="line-height: 0.5cm; margin-bottom: 0cm"><br/>

</p>
<h1 class="western">This implementation adds :</h1>
<ol>
	<li><p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
	<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">2
	local html pages which are accessible via WiFi. Just connect to the
	sensor in your WLAN and you're on the first page ...</span></font></font></font></p>
</ol>
<ul>
	<ul>
		<li><p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
		<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff"><b>first
		page</b> is showing actual sensor values (Dose rate, cps, HV
		pulses), Wifi-Data (Signal/Quality), free Memory, Firmware Version
		&amp; Date …<br/>
<br/>
</span></font></font></font><img src="README_Fork_changes_html_f3d2df7ae646bb5a.png" name="Bild1" align="left" width="366" height="412">
  <br clear="left"/>
</img>
<br/>

		</p>
		<li><p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
		<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">If
		BME280/680 is available, it also shows temperature, humidity &amp;
		pressure</span></font></font></font></p>
		<li><p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
		<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">the
		page refreshes automatically every 10s</span></font></font></font></p>
		<li><p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
		<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff"><b>second
		page</b> shows log infos which are normally only available through
		serial connection<br/>
<br/>
</span></font></font></font><img src="README_Fork_changes_html_2076c273428ddb7e.png" name="Bild2" align="left" width="575" height="433" border="0"/>
<br/>

		</p>
		<li><p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
		<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">the
		loginfo is updated automatically in the central frame and can be
		scrolled</span></font></font></font></p>
		<li><p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
		<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">The
		loglevel can be changed temporarily (runtime, not saved)<br/>
</span></font></font></font><br/>

		</p>
	</ul>
</ul>
<ol start="2">
	<li><p style="line-height: 0.5cm; margin-bottom: 0cm"><font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="font-weight: normal"><span style="background: #ffffff">a
	configurable http connection to a influx database included in the
	original configuration page<br/>
<br/>
</span></span></font></font></font><img src="README_Fork_changes_html_36ca21603972074b.png" name="Bild3" align="left" width="463" height="379" border="0"/>
<br/>

	</p>
	<li><p style="line-height: 0.5cm; margin-bottom: 0cm"><font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="font-weight: normal"><span style="background: #ffffff">a
	(extendable) translation system for the texts displayed on the local
	WLAN pages.</span></span></font></font></font></p>
</ol>
<ul>
	<ul>
		<li><p style="line-height: 0.5cm; margin-bottom: 0cm"><font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="font-weight: normal"><span style="background: #ffffff">Languages
		DE, EN and IT are available already.</span></span></font></font></font></p>
		<li><p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
		<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">Preferred
		language to be set on compile time</span></font></font></font></p>
	</ul>
</ul>
<ol start="4">
	<li><p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
	<font color="#000000"><span style="background: #ffffff"> <font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt">in
	the configuration page where Text attributes were showing only
	'?????' upon first run.</span></font></font></font></p>
	<p style="line-height: 0.5cm; margin-bottom: 0cm"></p>
</ol>
<h1 class="western">Compilation :</h1>
<p style="line-height: 0.5cm; margin-bottom: 0cm"><br/>

</p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">I
use 'MS code' and platformio for compilation, upload and monitoring
(I'm working under Fedora).</span></font></font></font></p>
<ul>
	<li><p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
	<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">You
	can copy the platformio-example.ini to platformio.ini, compile one
	of the (currently) 3 language options, and upload it.</span></font></font></font></p>
	<li><p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
	<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">The
	only thing to be adopted in platformio.ini is probably upload_port &amp;
	monitor_port</span></font></font></font></p>
	<li><p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
	<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">Necessary
	libraries should be pulled in automatically.</span></font></font></font></p>
	<li><p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
	<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">Furthermore
	you have to copy userdefines-example.h to userdefines.h and adopt
	the content before compilation.</span></font></font></font></p>
</ul>
<p style="line-height: 0.5cm; margin-bottom: 0cm"><br/>

</p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<font color="#000000"><font face="Droid Sans Mono, monospace, monospace"><font size="2" style="font-size: 10pt"><span style="background: #ffffff">I
have only experience with the WiFi version of this sensor, so the
pages will not be visible on the LoRaWan sensor option due to missing
WiFi (I guess).</span></font></font></font></p>
<p style="font-weight: normal; line-height: 0.5cm; margin-bottom: 0cm">
<br/>

</p>
<p style="line-height: 100%; margin-bottom: 0cm"><br/>

</p>
</body>
</html>