/*
    This file is part of Android File Transfer For Linux.
    Copyright (C) 2015  Vladimir Menshakov

    Android File Transfer For Linux is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    Android File Transfer For Linux is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Android File Transfer For Linux.
    If not, see <http://www.gnu.org/licenses/>.
 */

#include <usb/Context.h>
#include <usb/DeviceDescriptor.h>
#include <usb/call.h>
#include <stdio.h>

namespace
{

int usb_setup_device_iterator (io_iterator_t *deviceIterator, UInt32 location) {
  CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOUSBDeviceClassName);

  if (!matchingDict)
    return kIOReturnError;

  if (location) {
    CFMutableDictionaryRef propertyMatchDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                                                         &kCFTypeDictionaryKeyCallBacks,
                                                                         &kCFTypeDictionaryValueCallBacks);

    if (propertyMatchDict) {
      /* there are no unsigned CFNumber types so treat the value as signed. the os seems to do this
         internally (CFNumberType of locationID is 3) */
      CFTypeRef locationCF = CFNumberCreate (NULL, kCFNumberSInt32Type, &location);

      CFDictionarySetValue (propertyMatchDict, CFSTR(kUSBDevicePropertyLocationID), locationCF);
      /* release our reference to the CFNumber (CFDictionarySetValue retains it) */
      CFRelease (locationCF);

      CFDictionarySetValue (matchingDict, CFSTR(kIOPropertyMatchKey), propertyMatchDict);
      /* release out reference to the CFMutableDictionaryRef (CFDictionarySetValue retains it) */
      CFRelease (propertyMatchDict);
    }
    /* else we can still proceed as long as the caller accounts for the possibility of other devices in the iterator */
  }

  return IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, deviceIterator);
}

}

namespace mtp { namespace usb
{

	Context::Context()
	{
		io_iterator_t deviceIterator;

		USB_CALL(usb_setup_device_iterator (&deviceIterator, 0));

		io_service_t service;
		while ((service = IOIteratorNext (deviceIterator)))
		{
			try { _devices.push_back(std::make_shared<DeviceDescriptor>(service)); }
			catch(const std::exception &ex)
			{ fprintf(stderr, "%s\n", ex.what()); }

			IOObjectRelease(service);
		}

		IOObjectRelease(deviceIterator);
	}

	Context::~Context()
	{ }

	void Context::Wait()
	{ }


}}
