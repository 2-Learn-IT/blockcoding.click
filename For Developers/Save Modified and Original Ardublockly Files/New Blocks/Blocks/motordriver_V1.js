/**
 * WLAN Block!
 *
 * @license Licensed under the Apache License, Version 2.0 (the "License"):
 *          http://www.apache.org/licenses/LICENSE-2.0
 */

/**
 * @fileoverview Blocks for ESP WIFI library.
 * Code generator for the WIFI Connection setup.
 * Please Setup Serial before this Block to see all the status messages
 */
 
'use strict';

goog.provide('Blockly.Blocks.motordriver');

goog.require('Blockly.Blocks');
goog.require('Blockly.Types');

Blockly.Blocks['l298n_setup'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("Setup Motor Driver L298N - Select PINs for Motor:")
    this.appendDummyInput()
        .appendField("PWM PIN Motor A - Enable A")
		.appendField(new Blockly.FieldDropdown(Blockly.Arduino.Boards.selected.pwmPins), 'MotorA_enA');
    this.appendDummyInput()
        .appendField("GPIO PIN Motor A - Input 1")
		.appendField(new Blockly.FieldDropdown(Blockly.Arduino.Boards.selected.digitalPins), 'MotorA_in1');
    this.appendDummyInput()
        .appendField("GPIO PIN Motor A - Input 2")
		.appendField(new Blockly.FieldDropdown(Blockly.Arduino.Boards.selected.digitalPins), 'MotorA_in2');
    this.appendDummyInput()
        .appendField("GPIO PIN Motor B - Input 3")
		.appendField(new Blockly.FieldDropdown(Blockly.Arduino.Boards.selected.digitalPins), 'MotorB_in3');
    this.appendDummyInput()
        .appendField("GPIO PIN Motor B - Input 4")
		.appendField(new Blockly.FieldDropdown(Blockly.Arduino.Boards.selected.digitalPins), 'MotorB_in4');
    this.appendDummyInput()
        .appendField("PWM PIN Motor B - Enable B")
		.appendField(new Blockly.FieldDropdown(Blockly.Arduino.Boards.selected.pwmPins), 'MotorB_enB');
    this.setInputsInline(false);
	this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour(230);
 this.setTooltip("Motor Driver L298 PIN Setup including PWM for speed regulation");
 this.setHelpUrl("https://randomnerdtutorials.com/esp32-dc-motor-l298n-motor-driver-control-speed-direction/");
  }
};

Blockly.Blocks['l298n_move'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("Motor Driver L298N set Motor")
        .appendField(new Blockly.FieldDropdown([["A","MotorA"], ["B","MotorB"]]), "motor_select")
        .appendField("to")
        .appendField(new Blockly.FieldDropdown([["Stop","stop"], ["Forward","forward"], ["Backward","backward"]]), "movement");
    this.setInputsInline(false);
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour(230);
 this.setTooltip("\"l298_setup\" Block required before moving a motor");
 this.setHelpUrl("https://randomnerdtutorials.com/esp32-dc-motor-l298n-motor-driver-control-speed-direction/");
  }
};