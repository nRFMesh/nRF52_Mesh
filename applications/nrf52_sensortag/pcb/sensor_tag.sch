<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.0.0">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="24" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="88" name="SimResults" color="9" fill="1" visible="yes" active="yes"/>
<layer number="89" name="SimProbes" color="9" fill="1" visible="yes" active="yes"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="GY-BMEP">
<packages>
<package name="GY-BMEP">
<pad name="VIN" x="-5" y="3.81" drill="0.8"/>
<pad name="GND" x="-5" y="1.27" drill="0.8"/>
<pad name="SCL" x="-5" y="-1.27" drill="0.8"/>
<pad name="SDA" x="-5" y="-3.81" drill="0.8"/>
<wire x1="-6.58" y1="5.22" x2="-6.58" y2="-5.22" width="0.127" layer="21"/>
<wire x1="-6.58" y1="-5.22" x2="6.58" y2="-5.22" width="0.127" layer="21"/>
<wire x1="6.58" y1="-5.22" x2="6.58" y2="5.22" width="0.127" layer="21"/>
<wire x1="6.58" y1="5.22" x2="-6.58" y2="5.22" width="0.127" layer="21"/>
<hole x="3.81" y="2.54" drill="3"/>
<text x="0" y="-1.27" size="0.8128" layer="21">GY-BMEP</text>
<text x="-3.81" y="3.175" size="1.27" layer="21">VIN</text>
<text x="-3.81" y="0.635" size="1.27" layer="21">GND</text>
<text x="-3.81" y="-1.905" size="1.27" layer="21">SCL</text>
<text x="-3.81" y="-4.445" size="1.27" layer="21">SDA</text>
<text x="0" y="-3.81" size="0.8128" layer="21">BME280</text>
</package>
<package name="GY-BMEP-NOHOLE">
<pad name="VIN" x="-5" y="3.81" drill="0.8"/>
<pad name="GND" x="-5" y="1.27" drill="0.8"/>
<pad name="SCL" x="-5" y="-1.27" drill="0.8"/>
<pad name="SDA" x="-5" y="-3.81" drill="0.8"/>
<wire x1="-6.58" y1="5.22" x2="-6.58" y2="-5.22" width="0.127" layer="21"/>
<wire x1="-6.58" y1="-5.22" x2="6.58" y2="-5.22" width="0.127" layer="21"/>
<wire x1="6.58" y1="-5.22" x2="6.58" y2="5.22" width="0.127" layer="21"/>
<wire x1="6.58" y1="5.22" x2="-6.58" y2="5.22" width="0.127" layer="21"/>
<text x="0" y="-1.27" size="0.8128" layer="21">GY-BMEP</text>
<text x="-3.81" y="3.175" size="1.27" layer="21">VIN</text>
<text x="-3.81" y="0.635" size="1.27" layer="21">GND</text>
<text x="-3.81" y="-1.905" size="1.27" layer="21">SCL</text>
<text x="-3.81" y="-4.445" size="1.27" layer="21">SDA</text>
<text x="0" y="-3.81" size="0.8128" layer="21">BME280</text>
</package>
<package name="GY-BMEP-SMALL">
<wire x1="-2.77" y1="5.22" x2="-2.77" y2="-5.22" width="0.127" layer="21"/>
<wire x1="-2.77" y1="-5.22" x2="0.23" y2="-5.22" width="0.127" layer="21"/>
<wire x1="0.23" y1="-5.22" x2="0.23" y2="5.22" width="0.127" layer="21"/>
<wire x1="0.23" y1="5.22" x2="-2.77" y2="5.22" width="0.127" layer="21"/>
<text x="-2.413" y="2.54" size="0.8128" layer="21">V</text>
<text x="-2.413" y="0" size="0.8128" layer="21">G</text>
<text x="-2.54" y="-2.286" size="0.8128" layer="21">C</text>
<text x="-2.54" y="-4.826" size="0.8128" layer="21">D</text>
<pad name="VIN" x="-1.27" y="3.81" drill="0.8"/>
<pad name="GND" x="-1.27" y="1.27" drill="0.8"/>
<pad name="SCL" x="-1.27" y="-1.27" drill="0.8"/>
<pad name="SDA" x="-1.27" y="-3.81" drill="0.8"/>
</package>
</packages>
<symbols>
<symbol name="GY-BMEP">
<pin name="VIN" x="-5.08" y="7.62" length="middle"/>
<pin name="GND" x="-5.08" y="2.54" length="middle"/>
<pin name="SCL" x="-5.08" y="-2.54" length="middle"/>
<pin name="SDA" x="-5.08" y="-7.62" length="middle"/>
<wire x1="-2.54" y1="12.7" x2="-2.54" y2="-12.7" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-12.7" x2="38.1" y2="-12.7" width="0.254" layer="94"/>
<wire x1="38.1" y1="-12.7" x2="38.1" y2="12.7" width="0.254" layer="94"/>
<wire x1="38.1" y1="12.7" x2="-2.54" y2="12.7" width="0.254" layer="94"/>
<text x="22.86" y="2.54" size="1.778" layer="94">BME280</text>
<text x="22.86" y="0" size="1.778" layer="94">1.7 - 3.6 V</text>
<text x="22.86" y="7.62" size="1.778" layer="94">GY-BMEP</text>
<text x="10.16" y="-5.08" size="1.778" layer="94">Temperature</text>
<text x="10.16" y="-7.62" size="1.778" layer="94">Humidity</text>
<text x="10.16" y="-10.16" size="1.778" layer="94">Pressure</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="GY-BMEP">
<gates>
<gate name="G$1" symbol="GY-BMEP" x="-15.24" y="0"/>
</gates>
<devices>
<device name="" package="GY-BMEP">
<connects>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="SCL" pad="SCL"/>
<connect gate="G$1" pin="SDA" pad="SDA"/>
<connect gate="G$1" pin="VIN" pad="VIN"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="GY-BMEP-NOHOLE" package="GY-BMEP-NOHOLE">
<connects>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="SCL" pad="SCL"/>
<connect gate="G$1" pin="SDA" pad="SDA"/>
<connect gate="G$1" pin="VIN" pad="VIN"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="GY-BMEP-SMALL" package="GY-BMEP-SMALL">
<connects>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="SCL" pad="SCL"/>
<connect gate="G$1" pin="SDA" pad="SDA"/>
<connect gate="G$1" pin="VIN" pad="VIN"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="APDS9960">
<packages>
<package name="APDS9960-SMALL">
<pad name="VIN" x="0" y="5.08" drill="0.8"/>
<pad name="GND" x="0" y="2.54" drill="0.8"/>
<pad name="SCL" x="0" y="0" drill="0.8"/>
<pad name="SDA" x="0" y="-2.54" drill="0.8"/>
<pad name="INT" x="0" y="-5.08" drill="0.8"/>
<wire x1="-2.54" y1="6.35" x2="-2.54" y2="-6.35" width="0.127" layer="21"/>
<wire x1="-2.54" y1="-6.35" x2="1.27" y2="-6.35" width="0.127" layer="21"/>
<wire x1="1.27" y1="-6.35" x2="1.27" y2="6.35" width="0.127" layer="21"/>
<wire x1="1.27" y1="6.35" x2="-2.54" y2="6.35" width="0.127" layer="21"/>
<text x="-2.032" y="4.445" size="1.27" layer="21">V</text>
<text x="-2.032" y="1.905" size="1.27" layer="21">G</text>
<text x="-2.032" y="-0.635" size="1.27" layer="21">C</text>
<text x="-2.032" y="-3.175" size="1.27" layer="21">D</text>
<text x="-2.032" y="-5.715" size="1.27" layer="21">I</text>
</package>
<package name="APDS9960-FULL">
<pad name="VIN" x="0" y="5.08" drill="0.8"/>
<pad name="GND" x="0" y="2.54" drill="0.8"/>
<pad name="SCL" x="0" y="0" drill="0.8"/>
<pad name="SDA" x="0" y="-2.54" drill="0.8"/>
<pad name="INT" x="0" y="-5.08" drill="0.8"/>
<wire x1="-1.74" y1="6.35" x2="-1.74" y2="-6.35" width="0.127" layer="21"/>
<wire x1="-1.74" y1="-6.35" x2="11.3" y2="-6.35" width="0.127" layer="21"/>
<wire x1="11.3" y1="-6.35" x2="11.3" y2="6.35" width="0.127" layer="21"/>
<wire x1="11.3" y1="6.35" x2="-1.74" y2="6.35" width="0.127" layer="21"/>
<text x="1.778" y="4.445" size="1.27" layer="21">Vin</text>
<text x="1.778" y="1.905" size="1.27" layer="21">Gnd</text>
<text x="1.778" y="-0.635" size="1.27" layer="21">Scl</text>
<text x="1.778" y="-3.175" size="1.27" layer="21">Sda</text>
<text x="1.778" y="-5.715" size="1.27" layer="21">Int</text>
<text x="10.16" y="-5.08" size="1.4224" layer="21" rot="R90">APDS9960</text>
</package>
</packages>
<symbols>
<symbol name="APDS9960">
<pin name="VIN" x="-5.08" y="10.16" length="middle"/>
<pin name="GND" x="-5.08" y="5.08" length="middle"/>
<pin name="SCL" x="-5.08" y="0" length="middle"/>
<pin name="SDA" x="-5.08" y="-5.08" length="middle"/>
<pin name="INT" x="-5.08" y="-10.16" length="middle"/>
<wire x1="-2.54" y1="15.24" x2="-2.54" y2="-15.24" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-15.24" x2="38.1" y2="-15.24" width="0.254" layer="94"/>
<wire x1="38.1" y1="-15.24" x2="38.1" y2="15.24" width="0.254" layer="94"/>
<wire x1="38.1" y1="15.24" x2="-2.54" y2="15.24" width="0.254" layer="94"/>
<text x="15.24" y="-2.54" size="1.778" layer="94">APDS9960
Light
Colors
Proximity
Gesture</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="APDS9960_MODULE">
<description>This is the device for the APDS9960 module not the chip alone.</description>
<gates>
<gate name="G$1" symbol="APDS9960" x="-10.16" y="0"/>
</gates>
<devices>
<device name="" package="APDS9960-SMALL">
<connects>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="INT" pad="INT"/>
<connect gate="G$1" pin="SCL" pad="SCL"/>
<connect gate="G$1" pin="SDA" pad="SDA"/>
<connect gate="G$1" pin="VIN" pad="VIN"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="APDS9960-FULL" package="APDS9960-FULL">
<connects>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="INT" pad="INT"/>
<connect gate="G$1" pin="SCL" pad="SCL"/>
<connect gate="G$1" pin="SDA" pad="SDA"/>
<connect gate="G$1" pin="VIN" pad="VIN"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="nRF52832_module">
<packages>
<package name="NRF52832_MOD">
<wire x1="-8.2" y1="22.9" x2="8.2" y2="22.9" width="0.127" layer="21"/>
<wire x1="8.2" y1="22.9" x2="8.2" y2="0" width="0.127" layer="21"/>
<wire x1="8.2" y1="0" x2="-8.2" y2="0" width="0.127" layer="21"/>
<wire x1="-8.2" y1="0" x2="-8.2" y2="22.9" width="0.127" layer="21"/>
<smd name="P0_08" x="0" y="0" dx="2.54" dy="1.27" layer="1" roundness="100" rot="R90"/>
<smd name="P0_09" x="2.54" y="0" dx="2.54" dy="1.27" layer="1" roundness="100" rot="R90"/>
<smd name="P0_10" x="5.08" y="0" dx="2.54" dy="1.27" layer="1" roundness="100" rot="R90"/>
<smd name="P0_07" x="-2.54" y="0" dx="2.54" dy="1.27" layer="1" roundness="100" rot="R90"/>
<smd name="P0_06" x="-5.08" y="0" dx="2.54" dy="1.27" layer="1" roundness="100" rot="R90"/>
<smd name="P0_11" x="8.2" y="2.9" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="P0_12" x="8.2" y="5.44" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="P0_14" x="8.2" y="7.98" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="SWCLK" x="8.2" y="10.52" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="SWDIO" x="8.2" y="13.06" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="VCC" x="-8.2" y="2.54" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="GND" x="-8.2" y="5.44" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="P0_31" x="-8.2" y="7.98" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="P0_30" x="-8.2" y="10.52" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="P0_29" x="-8.2" y="13.06" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<wire x1="-4" y1="13.97" x2="-4" y2="21.59" width="1.27" layer="21"/>
<wire x1="-4" y1="21.59" x2="-3.81" y2="21.59" width="0.254" layer="21"/>
<wire x1="-3.81" y1="21.59" x2="-2.54" y2="21.59" width="0.508" layer="21"/>
<wire x1="-2.54" y1="21.59" x2="-1.27" y2="21.59" width="0.508" layer="21"/>
<wire x1="-1.27" y1="21.59" x2="-1.27" y2="17.78" width="0.508" layer="21"/>
<wire x1="-1.27" y1="17.78" x2="0" y2="17.78" width="0.508" layer="21"/>
<wire x1="0" y1="17.78" x2="0" y2="21.59" width="0.508" layer="21"/>
<wire x1="0" y1="21.59" x2="1.27" y2="21.59" width="0.508" layer="21"/>
<wire x1="1.27" y1="21.59" x2="1.27" y2="17.78" width="0.508" layer="21"/>
<wire x1="1.27" y1="17.78" x2="2.54" y2="17.78" width="0.508" layer="21"/>
<wire x1="2.54" y1="17.78" x2="2.54" y2="21.59" width="0.508" layer="21"/>
<wire x1="2.54" y1="21.59" x2="3.81" y2="21.59" width="0.508" layer="21"/>
<wire x1="3.81" y1="21.59" x2="3.81" y2="17.78" width="0.508" layer="21"/>
<wire x1="3.81" y1="17.78" x2="5.08" y2="17.78" width="0.508" layer="21"/>
<wire x1="5.08" y1="17.78" x2="5.08" y2="21.59" width="0.508" layer="21"/>
<wire x1="5.08" y1="21.59" x2="7.62" y2="21.59" width="0.508" layer="21"/>
<wire x1="-2.54" y1="21.59" x2="-2.54" y2="13.97" width="0.508" layer="21"/>
</package>
<package name="NRF52832_MOD_SMALL">
<wire x1="-8.2" y1="6.39" x2="8.2" y2="6.39" width="0.127" layer="21"/>
<wire x1="8.2" y1="6.39" x2="8.2" y2="-8.89" width="0.127" layer="21"/>
<wire x1="8.2" y1="-8.89" x2="-8.2" y2="-8.89" width="0.127" layer="21"/>
<wire x1="-8.2" y1="-8.89" x2="-8.2" y2="6.39" width="0.127" layer="21"/>
<smd name="P0_08" x="0" y="-8.89" dx="2.54" dy="1.27" layer="1" roundness="100" rot="R90"/>
<smd name="P0_09" x="2.54" y="-8.89" dx="2.54" dy="1.27" layer="1" roundness="100" rot="R90"/>
<smd name="P0_10" x="5.08" y="-8.89" dx="2.54" dy="1.27" layer="1" roundness="100" rot="R90"/>
<smd name="P0_07" x="-2.54" y="-8.89" dx="2.54" dy="1.27" layer="1" roundness="100" rot="R90"/>
<smd name="P0_06" x="-5.08" y="-8.89" dx="2.54" dy="1.27" layer="1" roundness="100" rot="R90"/>
<smd name="P0_11" x="8.2" y="-5.99" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="P0_12" x="8.2" y="-3.45" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="P0_14" x="8.2" y="-0.91" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="SWCLK" x="8.2" y="1.63" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="SWDIO" x="8.2" y="4.17" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="VCC" x="-8.2" y="-6.35" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="GND" x="-8.2" y="-3.45" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="P0_31" x="-8.2" y="-0.91" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="P0_30" x="-8.2" y="1.63" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<smd name="P0_29" x="-8.2" y="4.17" dx="2.54" dy="1.27" layer="1" roundness="100"/>
<text x="0" y="-3.81" size="1.778" layer="21" align="bottom-center">nRF52832
Module</text>
</package>
</packages>
<symbols>
<symbol name="NRF52832_MODULE">
<pin name="P0_29" x="-25.4" y="15.24" length="middle"/>
<pin name="P0_30" x="-25.4" y="10.16" length="middle"/>
<pin name="P0_31" x="-25.4" y="5.08" length="middle"/>
<pin name="GND" x="-25.4" y="0" length="middle"/>
<pin name="VCC" x="-25.4" y="-5.08" length="middle"/>
<pin name="P0_06" x="-10.16" y="-15.24" length="middle" rot="R90"/>
<pin name="P0_07" x="-5.08" y="-15.24" length="middle" rot="R90"/>
<pin name="P0_08" x="0" y="-15.24" length="middle" rot="R90"/>
<pin name="P0_09" x="5.08" y="-15.24" length="middle" rot="R90"/>
<pin name="P0_10" x="10.16" y="-15.24" length="middle" rot="R90"/>
<pin name="P0_11" x="25.4" y="-5.08" length="middle" rot="R180"/>
<pin name="P0_12" x="25.4" y="0" length="middle" rot="R180"/>
<pin name="P0_14" x="25.4" y="5.08" length="middle" rot="R180"/>
<pin name="SWCLK" x="25.4" y="10.16" length="middle" rot="R180"/>
<pin name="SWDIO" x="25.4" y="15.24" length="middle" rot="R180"/>
<wire x1="-22.86" y1="20.32" x2="-22.86" y2="-12.7" width="0.254" layer="94"/>
<wire x1="-22.86" y1="-12.7" x2="22.86" y2="-12.7" width="0.254" layer="94"/>
<wire x1="22.86" y1="-12.7" x2="22.86" y2="20.32" width="0.254" layer="94"/>
<wire x1="22.86" y1="20.32" x2="-22.86" y2="20.32" width="0.254" layer="94"/>
<text x="-7.62" y="5.08" size="2.54" layer="94">nRF52832
Module</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="NRF52832_MODULE">
<gates>
<gate name="G$1" symbol="NRF52832_MODULE" x="0" y="0"/>
</gates>
<devices>
<device name="" package="NRF52832_MOD">
<connects>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="P0_06" pad="P0_06"/>
<connect gate="G$1" pin="P0_07" pad="P0_07"/>
<connect gate="G$1" pin="P0_08" pad="P0_08"/>
<connect gate="G$1" pin="P0_09" pad="P0_09"/>
<connect gate="G$1" pin="P0_10" pad="P0_10"/>
<connect gate="G$1" pin="P0_11" pad="P0_11"/>
<connect gate="G$1" pin="P0_12" pad="P0_12"/>
<connect gate="G$1" pin="P0_14" pad="P0_14"/>
<connect gate="G$1" pin="P0_29" pad="P0_29"/>
<connect gate="G$1" pin="P0_30" pad="P0_30"/>
<connect gate="G$1" pin="P0_31" pad="P0_31"/>
<connect gate="G$1" pin="SWCLK" pad="SWCLK"/>
<connect gate="G$1" pin="SWDIO" pad="SWDIO"/>
<connect gate="G$1" pin="VCC" pad="VCC"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="NRF52832_MOD_SMALL" package="NRF52832_MOD_SMALL">
<connects>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="P0_06" pad="P0_06"/>
<connect gate="G$1" pin="P0_07" pad="P0_07"/>
<connect gate="G$1" pin="P0_08" pad="P0_08"/>
<connect gate="G$1" pin="P0_09" pad="P0_09"/>
<connect gate="G$1" pin="P0_10" pad="P0_10"/>
<connect gate="G$1" pin="P0_11" pad="P0_11"/>
<connect gate="G$1" pin="P0_12" pad="P0_12"/>
<connect gate="G$1" pin="P0_14" pad="P0_14"/>
<connect gate="G$1" pin="P0_29" pad="P0_29"/>
<connect gate="G$1" pin="P0_30" pad="P0_30"/>
<connect gate="G$1" pin="P0_31" pad="P0_31"/>
<connect gate="G$1" pin="SWCLK" pad="SWCLK"/>
<connect gate="G$1" pin="SWDIO" pad="SWDIO"/>
<connect gate="G$1" pin="VCC" pad="VCC"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="cr2032">
<packages>
<package name="CR2032_SMD">
<smd name="VCC_R" x="11.43" y="0" dx="7.2" dy="2" layer="1" rot="R90"/>
<smd name="VCC_L" x="-11.43" y="0" dx="7.2" dy="2" layer="1" rot="R90"/>
<smd name="GND" x="0" y="0" dx="16.9" dy="16.9" layer="1" roundness="100"/>
<text x="-11.43" y="3.81" size="1.27" layer="21">V+</text>
<text x="10.16" y="3.81" size="1.27" layer="21">V+</text>
<text x="-1.905" y="-9.779" size="1.27" layer="21">GND</text>
<circle x="0" y="0" radius="10" width="0.127" layer="21"/>
</package>
</packages>
<symbols>
<symbol name="CR2032">
<pin name="VCC" x="-5.08" y="5.08" length="middle"/>
<pin name="GND" x="-5.08" y="-5.08" length="middle"/>
<text x="0.254" y="-1.778" size="1.778" layer="94">CR2032
small coin cell</text>
<circle x="7.62" y="0" radius="11.359225" width="0.254" layer="94"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="CR2032">
<gates>
<gate name="G$1" symbol="CR2032" x="-7.62" y="0"/>
</gates>
<devices>
<device name="" package="CR2032_SMD">
<connects>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="VCC" pad="VCC_L VCC_R"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="nrf_tag_debug">
<packages>
<package name="NRF_TAG_DEBUG">
<wire x1="-8.89" y1="2.54" x2="-8.89" y2="-2.54" width="0.127" layer="21"/>
<wire x1="-8.89" y1="-2.54" x2="8.89" y2="-2.54" width="0.127" layer="21"/>
<wire x1="8.89" y1="-2.54" x2="8.89" y2="2.54" width="0.127" layer="21"/>
<wire x1="8.89" y1="2.54" x2="-8.89" y2="2.54" width="0.127" layer="21"/>
<text x="-8.509" y="1.143" size="1.27" layer="21">Debug</text>
<text x="-8.509" y="-2.159" size="1.27" layer="21">Tx</text>
<text x="-0.635" y="-2.159" size="1.27" layer="21">G</text>
<text x="2.032" y="-2.159" size="1.27" layer="21">V</text>
<text x="4.572" y="-2.159" size="1.27" layer="21">D</text>
<text x="7.112" y="-2.159" size="1.27" layer="21">C</text>
<pad name="TX" x="-7.62" y="0" drill="0.6"/>
<pad name="RX" x="-5.08" y="0" drill="0.6"/>
<pad name="GND" x="0" y="0" drill="0.6"/>
<pad name="VCC" x="2.54" y="0" drill="0.6"/>
<pad name="SWDIO" x="5.08" y="0" drill="0.6"/>
<pad name="SWDCLK" x="7.62" y="0" drill="0.6"/>
<pad name="NC" x="-2.54" y="0" drill="0.6" shape="square"/>
<text x="-5.969" y="-2.159" size="1.27" layer="21">Rx</text>
</package>
</packages>
<symbols>
<symbol name="NRF_TAG_DEBUG">
<pin name="RX" x="-5.08" y="10.16" length="middle"/>
<pin name="GND" x="-5.08" y="5.08" length="middle"/>
<pin name="VCC" x="-5.08" y="0" length="middle"/>
<pin name="SWDIO" x="-5.08" y="-5.08" length="middle"/>
<pin name="SWCLK" x="-5.08" y="-10.16" length="middle"/>
<wire x1="-2.54" y1="17.78" x2="-2.54" y2="-12.7" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-12.7" x2="17.78" y2="-12.7" width="0.254" layer="94"/>
<wire x1="17.78" y1="-12.7" x2="17.78" y2="17.78" width="0.254" layer="94"/>
<wire x1="17.78" y1="17.78" x2="-2.54" y2="17.78" width="0.254" layer="94"/>
<text x="6.604" y="7.366" size="2.54" layer="94">Debug</text>
<pin name="TX" x="-5.08" y="15.24" length="middle"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="NRF_TAG_DEBUG">
<gates>
<gate name="G$1" symbol="NRF_TAG_DEBUG" x="-7.62" y="0"/>
</gates>
<devices>
<device name="" package="NRF_TAG_DEBUG">
<connects>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="RX" pad="RX"/>
<connect gate="G$1" pin="SWCLK" pad="SWDCLK"/>
<connect gate="G$1" pin="SWDIO" pad="SWDIO"/>
<connect gate="G$1" pin="TX" pad="TX"/>
<connect gate="G$1" pin="VCC" pad="VCC"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="cr2477">
<packages>
<package name="CR2477">
<pad name="GND" x="0" y="0" drill="0.8"/>
<pad name="VCC" x="19" y="0" drill="0.8"/>
<circle x="3.81" y="0" radius="13.2" width="0.127" layer="21"/>
<wire x1="16.002" y1="5.24" x2="20.32" y2="5.23" width="0.127" layer="21"/>
<wire x1="20.32" y1="5.23" x2="20.32" y2="-5.23" width="0.127" layer="21"/>
<wire x1="20.32" y1="-5.23" x2="16.002" y2="-5.24" width="0.127" layer="21"/>
<text x="-9.144" y="-0.508" size="1.27" layer="21">CR2477</text>
</package>
</packages>
<symbols>
<symbol name="CR2477">
<pin name="VCC" x="-12.7" y="5.08" length="middle"/>
<pin name="GND" x="-12.7" y="-5.08" length="middle"/>
<text x="-7.366" y="-1.778" size="1.778" layer="94">CR2477
Big coin cell</text>
<circle x="0" y="0" radius="13" width="0.254" layer="94"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="CR2477">
<gates>
<gate name="G$1" symbol="CR2477" x="0" y="0"/>
</gates>
<devices>
<device name="" package="CR2477">
<connects>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="VCC" pad="VCC"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="U$1" library="GY-BMEP" deviceset="GY-BMEP" device="GY-BMEP-NOHOLE" value="GY-BMEPGY-BMEP-NOHOLE"/>
<part name="U$2" library="APDS9960" deviceset="APDS9960_MODULE" device="APDS9960-FULL" value="APDS9960_MODULEAPDS9960-FULL"/>
<part name="U$3" library="nRF52832_module" deviceset="NRF52832_MODULE" device="NRF52832_MOD_SMALL" value="NRF52832_MODULENRF52832_MOD_SMALL"/>
<part name="U$4" library="cr2032" deviceset="CR2032" device=""/>
<part name="U$5" library="nrf_tag_debug" deviceset="NRF_TAG_DEBUG" device=""/>
<part name="U$6" library="cr2477" deviceset="CR2477" device=""/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="U$1" gate="G$1" x="33.02" y="12.7"/>
<instance part="U$2" gate="G$1" x="33.02" y="58.42"/>
<instance part="U$3" gate="G$1" x="-48.26" y="50.8"/>
<instance part="U$4" gate="G$1" x="-7.62" y="-5.08"/>
<instance part="U$5" gate="G$1" x="-68.58" y="0" rot="R180"/>
<instance part="U$6" gate="G$1" x="33.02" y="-17.78"/>
</instances>
<busses>
</busses>
<nets>
<net name="SWCLK" class="0">
<segment>
<pinref part="U$5" gate="G$1" pin="SWCLK"/>
<wire x1="-63.5" y1="10.16" x2="-25.4" y2="10.16" width="0.1524" layer="91"/>
<wire x1="-25.4" y1="10.16" x2="-25.4" y2="22.86" width="0.1524" layer="91"/>
<wire x1="-25.4" y1="22.86" x2="-7.62" y2="22.86" width="0.1524" layer="91"/>
<pinref part="U$3" gate="G$1" pin="SWCLK"/>
<wire x1="-7.62" y1="22.86" x2="-7.62" y2="60.96" width="0.1524" layer="91"/>
<wire x1="-7.62" y1="60.96" x2="-22.86" y2="60.96" width="0.1524" layer="91"/>
<label x="-14.732" y="61.976" size="1.778" layer="95"/>
</segment>
</net>
<net name="SWDIO" class="0">
<segment>
<pinref part="U$3" gate="G$1" pin="SWDIO"/>
<wire x1="-22.86" y1="66.04" x2="-5.08" y2="66.04" width="0.1524" layer="91"/>
<wire x1="-5.08" y1="66.04" x2="-5.08" y2="20.32" width="0.1524" layer="91"/>
<wire x1="-5.08" y1="20.32" x2="-22.86" y2="20.32" width="0.1524" layer="91"/>
<wire x1="-22.86" y1="20.32" x2="-22.86" y2="5.08" width="0.1524" layer="91"/>
<pinref part="U$5" gate="G$1" pin="SWDIO"/>
<wire x1="-22.86" y1="5.08" x2="-63.5" y2="5.08" width="0.1524" layer="91"/>
<label x="-14.986" y="67.564" size="1.778" layer="95"/>
</segment>
</net>
<net name="GND" class="0">
<segment>
<pinref part="U$4" gate="G$1" pin="GND"/>
<pinref part="U$5" gate="G$1" pin="GND"/>
<wire x1="-63.5" y1="-5.08" x2="-30.48" y2="-5.08" width="0.1524" layer="91"/>
<wire x1="-30.48" y1="-5.08" x2="-30.48" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="-30.48" y1="-10.16" x2="-17.78" y2="-10.16" width="0.1524" layer="91"/>
<label x="-30.48" y="-10.16" size="1.778" layer="95" rot="R180"/>
<wire x1="-17.78" y1="-10.16" x2="-12.7" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="-17.78" y1="-10.16" x2="-17.78" y2="-22.86" width="0.1524" layer="91"/>
<pinref part="U$6" gate="G$1" pin="GND"/>
<wire x1="-17.78" y1="-22.86" x2="20.32" y2="-22.86" width="0.1524" layer="91"/>
<junction x="-17.78" y="-10.16"/>
</segment>
<segment>
<pinref part="U$3" gate="G$1" pin="GND"/>
<wire x1="-73.66" y1="50.8" x2="-81.28" y2="50.8" width="0.1524" layer="91"/>
<label x="-83.82" y="50.8" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="U$2" gate="G$1" pin="GND"/>
<wire x1="27.94" y1="63.5" x2="20.32" y2="63.5" width="0.1524" layer="91"/>
<label x="20.32" y="63.5" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="U$1" gate="G$1" pin="GND"/>
<wire x1="27.94" y1="15.24" x2="22.86" y2="15.24" width="0.1524" layer="91"/>
<label x="20.32" y="15.24" size="1.778" layer="95"/>
</segment>
</net>
<net name="VCC" class="0">
<segment>
<pinref part="U$4" gate="G$1" pin="VCC"/>
<pinref part="U$5" gate="G$1" pin="VCC"/>
<wire x1="-12.7" y1="0" x2="-20.32" y2="0" width="0.1524" layer="91"/>
<label x="-16.002" y="-1.778" size="1.778" layer="95" rot="R180"/>
<pinref part="U$6" gate="G$1" pin="VCC"/>
<wire x1="-20.32" y1="0" x2="-63.5" y2="0" width="0.1524" layer="91"/>
<wire x1="20.32" y1="-12.7" x2="12.7" y2="-12.7" width="0.1524" layer="91"/>
<wire x1="12.7" y1="-12.7" x2="12.7" y2="7.62" width="0.1524" layer="91"/>
<wire x1="12.7" y1="7.62" x2="-20.32" y2="7.62" width="0.1524" layer="91"/>
<wire x1="-20.32" y1="7.62" x2="-20.32" y2="0" width="0.1524" layer="91"/>
<junction x="-20.32" y="0"/>
</segment>
<segment>
<pinref part="U$3" gate="G$1" pin="VCC"/>
<wire x1="-73.66" y1="45.72" x2="-81.28" y2="45.72" width="0.1524" layer="91"/>
<label x="-83.82" y="45.72" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="U$2" gate="G$1" pin="VIN"/>
<wire x1="27.94" y1="68.58" x2="20.32" y2="68.58" width="0.1524" layer="91"/>
<label x="20.32" y="71.12" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="U$1" gate="G$1" pin="VIN"/>
<wire x1="27.94" y1="20.32" x2="22.86" y2="20.32" width="0.1524" layer="91"/>
<label x="22.86" y="20.32" size="1.778" layer="95"/>
</segment>
</net>
<net name="SCL" class="0">
<segment>
<pinref part="U$1" gate="G$1" pin="SCL"/>
<wire x1="27.94" y1="10.16" x2="15.24" y2="10.16" width="0.1524" layer="91"/>
<wire x1="15.24" y1="10.16" x2="15.24" y2="58.42" width="0.1524" layer="91"/>
<pinref part="U$2" gate="G$1" pin="SCL"/>
<wire x1="15.24" y1="58.42" x2="27.94" y2="58.42" width="0.1524" layer="91"/>
<wire x1="15.24" y1="58.42" x2="12.7" y2="58.42" width="0.1524" layer="91"/>
<wire x1="12.7" y1="58.42" x2="12.7" y2="76.2" width="0.1524" layer="91"/>
<wire x1="12.7" y1="76.2" x2="-78.74" y2="76.2" width="0.1524" layer="91"/>
<pinref part="U$3" gate="G$1" pin="P0_29"/>
<wire x1="-78.74" y1="76.2" x2="-78.74" y2="66.04" width="0.1524" layer="91"/>
<wire x1="-78.74" y1="66.04" x2="-73.66" y2="66.04" width="0.1524" layer="91"/>
<junction x="15.24" y="58.42"/>
<label x="-83.82" y="66.294" size="1.778" layer="95"/>
</segment>
</net>
<net name="SDA" class="0">
<segment>
<pinref part="U$2" gate="G$1" pin="SDA"/>
<wire x1="27.94" y1="53.34" x2="17.78" y2="53.34" width="0.1524" layer="91"/>
<wire x1="17.78" y1="53.34" x2="17.78" y2="5.08" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="SDA"/>
<wire x1="17.78" y1="5.08" x2="27.94" y2="5.08" width="0.1524" layer="91"/>
<wire x1="17.78" y1="53.34" x2="7.62" y2="53.34" width="0.1524" layer="91"/>
<wire x1="7.62" y1="53.34" x2="7.62" y2="73.66" width="0.1524" layer="91"/>
<wire x1="7.62" y1="73.66" x2="-86.36" y2="73.66" width="0.1524" layer="91"/>
<wire x1="-86.36" y1="73.66" x2="-86.36" y2="60.96" width="0.1524" layer="91"/>
<pinref part="U$3" gate="G$1" pin="P0_30"/>
<wire x1="-86.36" y1="60.96" x2="-73.66" y2="60.96" width="0.1524" layer="91"/>
<junction x="17.78" y="53.34"/>
<label x="-91.694" y="61.976" size="1.778" layer="95"/>
</segment>
</net>
<net name="INT" class="0">
<segment>
<pinref part="U$3" gate="G$1" pin="P0_31"/>
<wire x1="-73.66" y1="55.88" x2="-88.9" y2="55.88" width="0.1524" layer="91"/>
<wire x1="-88.9" y1="55.88" x2="-88.9" y2="15.24" width="0.1524" layer="91"/>
<wire x1="-88.9" y1="15.24" x2="10.16" y2="15.24" width="0.1524" layer="91"/>
<wire x1="10.16" y1="15.24" x2="10.16" y2="48.26" width="0.1524" layer="91"/>
<pinref part="U$2" gate="G$1" pin="INT"/>
<wire x1="10.16" y1="48.26" x2="27.94" y2="48.26" width="0.1524" layer="91"/>
<label x="-83.058" y="56.896" size="1.778" layer="95"/>
</segment>
</net>
<net name="TX" class="0">
<segment>
<pinref part="U$3" gate="G$1" pin="P0_06"/>
<pinref part="U$5" gate="G$1" pin="TX"/>
<wire x1="-58.42" y1="35.56" x2="-58.42" y2="-15.24" width="0.1524" layer="91"/>
<wire x1="-58.42" y1="-15.24" x2="-63.5" y2="-15.24" width="0.1524" layer="91"/>
<label x="-61.976" y="28.956" size="1.778" layer="95"/>
</segment>
</net>
<net name="RX" class="0">
<segment>
<pinref part="U$3" gate="G$1" pin="P0_07"/>
<wire x1="-53.34" y1="35.56" x2="-53.34" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="U$5" gate="G$1" pin="RX"/>
<wire x1="-53.34" y1="-10.16" x2="-63.5" y2="-10.16" width="0.1524" layer="91"/>
<label x="-52.324" y="28.702" size="1.778" layer="95"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
<compatibility>
<note version="6.3" minversion="6.2.2" severity="warning">
Since Version 6.2.2 text objects can contain more than one line,
which will not be processed correctly with this version.
</note>
</compatibility>
</eagle>
