/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

/***********************************************************************************
  SNES9X for Mac OS (c) Copyright John Stiles

  Snes9x for Mac OS X

  (c) Copyright 2001 - 2011  zones
  (c) Copyright 2002 - 2005  107
  (c) Copyright 2002         PB1400c
  (c) Copyright 2004         Alexander and Sander
  (c) Copyright 2004 - 2005  Steven Seeger
  (c) Copyright 2005         Ryan Vogt
  (c) Copyright 2019         Michael Donald Buckley
 ***********************************************************************************/

#import <snes9x_framework/snes9x_framework.h>
#import "AppDelegate.h"
#import "S9xPreferencesWindowController.h"
#import "S9xButtonConfigTextField.h"

NSString * const kKeyboardPrefs = @"KeyboardConfig";
NSString * const kJoypadInputPrefs = @"JoypadInputs";
NSString * const kJoypadPlayerPrefs = @"JoypadPlayers";
NSString * const kShowFPSPref = @"ShowFPS";
NSString * const kVideoModePref = @"VideoMode";
NSString * const kMacFrameSkipPref = @"FrameSkip";

@implementation S9xPreferencesWindowController
@synthesize tabView, videoModePopup, showFPSCheckbox, devicePopUp, playerPopUp, macFrameSkipTextField, macFrameSkipStepper, macFrameSkipAutomaticButton, configTextFields;

- (void)windowDidLoad
{
    [self refresh];
}

- (void)awakeFromNib
{
    AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
    NSUInteger joypadIndex = 0;
    for (S9xJoypad *joypad in [appDelegate listJoypads])
    {
        NSMenuItem *item = [NSMenuItem new];
        item.title = joypad.name;
        item.tag = joypadIndex++;
        item.representedObject = joypad;
        [self.devicePopUp.menu addItem:item];
    }

	// collect all S9xButtonConfigTextFields within subviews
	NSMutableArray *configTextFields = [[NSMutableArray alloc] init];
	for (NSTabViewItem *tabViewItem in self.tabView.tabViewItems) {
		[configTextFields addObjectsFromArray:[self getS9xButtonConfigTextFields:tabViewItem.view]];
	}
	self.configTextFields = configTextFields;

	for (S9xButtonConfigTextField *configTextField in self.configTextFields)
	{
		[configTextField addObserver:self forKeyPath:@"keyCode" options:NSKeyValueObservingOptionNew context:NULL];
		[configTextField addObserver:self forKeyPath:@"joypadInput" options:NSKeyValueObservingOptionNew context:NULL];
	}
	
	// select Keyboard as default
    [self selectDeviceForPlayer:0];
}

- (void)refresh
{
    NSUInteger index = MIN([NSUserDefaults.standardUserDefaults integerForKey:kVideoModePref], 1);
    [self.videoModePopup selectItemAtIndex:index];
	
    NSInteger macFrameSkipDefault = [NSUserDefaults.standardUserDefaults integerForKey:kMacFrameSkipPref];
	// if macFrameSkip is equal to -1, set automatic checkbox
	// to be checked, disable the ability to change frame skip values
	// from stepper/text field, else leave automatic checkbox
	// unchecked and set textfield to value
	if (macFrameSkipDefault == -1) {
		[self.macFrameSkipTextField setEnabled: false];
		[self.macFrameSkipStepper setEnabled: false];
		[self.macFrameSkipTextField setIntValue: 0]; // show something at least
		[self.macFrameSkipAutomaticButton setIntValue: 1];
	} else {
		[self.macFrameSkipTextField setIntValue: (int)macFrameSkipDefault];
		[self.macFrameSkipAutomaticButton setIntValue: 0];
	}
	
    self.showFPSCheckbox.state = [NSUserDefaults.standardUserDefaults boolForKey:kShowFPSPref];

    if (self.devicePopUp.selectedItem.tag < 0)
    {
        NSMutableDictionary<NSNumber *, NSNumber *> *controlsDict = [NSMutableDictionary new];

        NSDictionary *keyboardDict = [NSUserDefaults.standardUserDefaults objectForKey:kKeyboardPrefs];
        NSInteger playerNum = self.playerPopUp.selectedItem.tag;

        for (NSUInteger i = 0; i < kNumButtons; ++i)
        {
            controlsDict[@(i)] = keyboardDict[@(i + (playerNum * kNumButtons)).stringValue];
        }

        for (S9xButtonConfigTextField *configTextField in self.configTextFields)
        {
			[configTextField removeObserver:self forKeyPath:@"keyCode"];
			[configTextField removeObserver:self forKeyPath:@"joypadInput"];

			NSNumber *keyCode = controlsDict[@(configTextField.tag)];

			configTextField.joypadInput = nil;

			if ( keyCode != nil )
			{
				configTextField.keyCode = keyCode.intValue;
			}
			else
			{
				configTextField.keyCode = -1;
			}

			[configTextField addObserver:self forKeyPath:@"keyCode" options:NSKeyValueObservingOptionNew context:NULL];
			[configTextField addObserver:self forKeyPath:@"joypadInput" options:NSKeyValueObservingOptionNew context:NULL];

			configTextField.disableKeyboardInput = NO;
        }
    }
    else
    {
        AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
        S9xJoypad *joypad = self.devicePopUp.selectedItem.representedObject;
        NSString *joypadKey = [appDelegate prefsKeyForVendorID:joypad.vendorID productID:joypad.productID index:joypad.index];
        NSDictionary *joypadDIct = [[NSUserDefaults.standardUserDefaults objectForKey:kJoypadInputPrefs] objectForKey:joypadKey];

        for (S9xButtonConfigTextField *configTextField in self.configTextFields)
        {
            [configTextField removeObserver:self forKeyPath:@"keyCode"];
			[configTextField removeObserver:self forKeyPath:@"joypadInput"];

			uint32 cookie = 0;
			int32 value = 0;
			S9xButtonCode buttonCode = (S9xButtonCode)configTextField.tag;
			NSString *inputString = joypadDIct[@(buttonCode).stringValue];

			configTextField.keyCode = -1;

			if ([appDelegate getValuesFromString:inputString cookie:&cookie value:&value])
			{
				S9xJoypadInput *input = [S9xJoypadInput new];
				input.cookie = cookie;
				input.value = value;
				input.buttonCode = buttonCode;
				configTextField.joypadInput = input;
				configTextField.stringValue = [appDelegate labelForVendorID:joypad.vendorID productID:joypad.productID cookie:cookie value:value];
			}
			else
			{
				configTextField.joypadInput = nil;
				configTextField.stringValue = @"";
			}

			[configTextField addObserver:self forKeyPath:@"keyCode" options:NSKeyValueObservingOptionNew context:NULL];
			[configTextField addObserver:self forKeyPath:@"joypadInput" options:NSKeyValueObservingOptionNew context:NULL];

			configTextField.disableKeyboardInput = YES;
        }
    }
}

- (NSArray *)getS9xButtonConfigTextFields:(NSView*)view {
    NSMutableArray *S9xButtonConfigTextFields = [[NSMutableArray alloc] init];
	
	if ([view isKindOfClass:[S9xButtonConfigTextField class]]) {
		[S9xButtonConfigTextFields addObject:view];
	} else if (view.subviews) {
		for (NSView *subview in view.subviews) {
			[S9xButtonConfigTextFields addObjectsFromArray:[self getS9xButtonConfigTextFields:subview]];
		}
	}
    return [NSArray arrayWithArray:S9xButtonConfigTextFields];
}

- (void)selectDeviceForPlayer:(int8_t)player
{
    AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
    NSString* joypadKey = [[NSUserDefaults.standardUserDefaults objectForKey:kJoypadPlayerPrefs] objectForKey:@(player).stringValue];

    [self.devicePopUp selectItemAtIndex:0];

    if (joypadKey != nil)
    {
        uint32 vendorID = 0;
        uint32 productID = 0;
        uint32 index = 0;

        if ( [appDelegate getValuesFromString:joypadKey vendorID:&vendorID productID:&productID index:&index])
        {
            S9xJoypad *joypad = [S9xJoypad new];
            joypad.vendorID = vendorID;
            joypad.productID = productID;
            joypad.index = index;

            for (NSMenuItem *item in self.devicePopUp.menu.itemArray)
            {
                if ([joypad isEqual:item.representedObject])
                {
                    [self.devicePopUp selectItem:item];
                    break;
                }
            }
        }
    }
}

- (BOOL)handleInput:(S9xJoypadInput *)input fromJoypad:(S9xJoypad *)joypad
{
    id firstResponder = self.window.firstResponder;

    if ([firstResponder respondsToSelector:@selector(isFieldEditor)] && [firstResponder isFieldEditor])
    {
        firstResponder = [firstResponder delegate];
    }

    if ([firstResponder respondsToSelector:@selector(setJoypadInput:)])
    {
        S9xJoypad *currentJoypad = self.devicePopUp.selectedItem.representedObject;

        if ([joypad isEqual:currentJoypad])
        {
            [firstResponder setJoypadInput:input];
            return YES;
        }
    }

    return NO;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"keyCode"])
    {
        S9xButtonConfigTextField *field = (S9xButtonConfigTextField *)object;
        S9xButtonCode buttonCode = (S9xButtonCode)field.tag;
        uint16_t keyCode = field.keyCode;
        int8_t player = self.playerPopUp.selectedItem.tag;

        if (keyCode != (CGKeyCode)-1)
        {
            [((AppDelegate *) NSApp.delegate) setButtonCode:buttonCode forKeyCode:keyCode player:player];
        }
        else
        {
            [((AppDelegate *) NSApp.delegate) clearButton:buttonCode forPlayer:player];
        }

        [NSUserDefaults.standardUserDefaults synchronize];

        [self refresh];
    }
    else if ( [keyPath isEqualToString:@"joypadInput"])
    {
        S9xButtonConfigTextField *field = (S9xButtonConfigTextField *)object;
        S9xButtonCode buttonCode = (S9xButtonCode)field.tag;
        S9xJoypad *joypad = self.devicePopUp.selectedItem.representedObject;

        if ([joypad isKindOfClass:[S9xJoypad class]])
        {
            S9xJoypadInput *input = field.joypadInput;

            if (input != nil)
            {
                [((AppDelegate *)NSApp.delegate) setButton:buttonCode forVendorID:joypad.vendorID productID:joypad.productID index:joypad.index cookie:input.cookie value:input.value];
            }
            else
            {
                [((AppDelegate *)NSApp.delegate) clearJoypadForVendorID:joypad.vendorID productID:joypad.productID index:joypad.index buttonCode:buttonCode];
            }
        }

        [NSUserDefaults.standardUserDefaults synchronize];
        [self refresh];
    }
}

- (void)setShowFPS:(BOOL)value
{
    AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
    [appDelegate setShowFPS:value];
}

- (void)setVideoMode:(int)value
{
    AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
    [appDelegate setVideoMode:value];
}

- (void)setMacFrameSkip:(int)value
{
    AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
	[appDelegate setMacFrameSkip:value];
}

- (IBAction)onToolbarItemPress:(NSToolbarItem *)sender
{
	[self.tabView selectTabViewItemAtIndex:sender.tag];
}

- (IBAction)onCheckShowFPS:(NSButton *)sender
{
	[self setShowFPS:sender.state == NSOnState];
}

- (IBAction)onSelectVideoMode:(NSPopUpButton *)sender
{
    [self setVideoMode:(int)sender.selectedTag];
}

- (IBAction)bumpMacFrameSkip:(NSStepper *)sender
{
	int bumpValue = sender.intValue;   // 1 or -1
	int nextValue = self.macFrameSkipTextField.intValue + bumpValue;
	
	// constrain value
	if (nextValue < 0) {
		nextValue = 0;
	}
	if (nextValue > 200) {
		nextValue = 200;
	}
    
	[self.macFrameSkipTextField setIntValue: nextValue];
	[sender setIntValue:0];	// reset stepper value
	[self setMacFrameSkip:self.macFrameSkipTextField.intValue]; // execute setter
}

- (IBAction)onChangeMacFrameSkipTextField:(NSTextField *)sender
{
	[self setMacFrameSkip:sender.intValue];
}

- (IBAction)onCheckMacFrameSkipAutomaticButton:(NSButton *)sender
{
	if (sender.intValue == 1) {
		// when automatic is checked, disable macFrameSkipTextField and
		// macFrameSkipStepper, then set macFrameSkip to -1 (automatic)
		[self.macFrameSkipTextField setEnabled:false];
		[self.macFrameSkipStepper setEnabled:false];
		[self setMacFrameSkip:-1];
	} else {
		// when automatic is unchecked, enable macFrameSkipTextField and
		// macFrameSkipStepper, then set macFrameSkip to value of text field
		[self.macFrameSkipTextField setEnabled:true];
		[self.macFrameSkipStepper setEnabled:true];
		[self setMacFrameSkip:self.macFrameSkipTextField.intValue];
	}
}

- (IBAction)onChangePlayerDropdown:(NSPopUpButton *)sender
{
    [self selectDeviceForPlayer:sender.selectedTag];
    [self refresh];
}

- (IBAction)onChangeDeviceDropdown:(NSPopUpButton *)sender
{
    if (sender.selectedTag >= 0)
    {
        AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
        S9xJoypad *joypad = sender.selectedItem.representedObject;
        [appDelegate setPlayer:self.playerPopUp.selectedTag forVendorID:joypad.vendorID productID:joypad.productID index:joypad.index];
        [NSUserDefaults.standardUserDefaults synchronize];
    }
    [self refresh];
}
@end
