/*
 * Copyright (C) 2015 Jared Boone, ShareBrained Technology, Inc.
 * Copyright (C) 2017 Furrtek
 * 
 * This file is part of PortaPack.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "ui.hpp"
#include "file.hpp"
#include "ui_receiver.hpp"
#include "ui_font_fixed_8x16.hpp"
#include "recent_entries.hpp"

#include "adsb.hpp"
#include "message.hpp"

using namespace adsb;

namespace ui {

struct AircraftRecentEntry {
	using Key = uint32_t;
	
	static constexpr Key invalid_key = 0xffffffff;
	
	uint32_t ICAO_address { };
	uint16_t hits { 0 };
	adsb_pos pos { false, 0, 0, 0 };
	
	ADSBFrame frame_pos_even { };
	ADSBFrame frame_pos_odd { };
	
	std::string callsign { "        " };
	std::string time_string { "" };
	std::string pos_string { "" };
	
	AircraftRecentEntry(
		const uint32_t ICAO_address
	) : ICAO_address { ICAO_address }
	{
	}

	Key key() const {
		return ICAO_address;
	}
	
	void set_callsign(std::string& new_callsign) {
		callsign = new_callsign;
	}
	
	void inc_hit() {
		hits++;
	}
	
	void set_frame_pos(ADSBFrame& frame, uint32_t parity) {
		if (!parity)
			frame_pos_even = frame;
		else
			frame_pos_odd = frame;
		
		if (!frame_pos_even.empty() && !frame_pos_odd.empty()) {
			if (abs(frame_pos_even.get_rx_timestamp() - frame_pos_odd.get_rx_timestamp()) < 20)
				pos = decode_frame_pos(frame_pos_even, frame_pos_odd);
		}
	}
	
	void set_pos_string(std::string& new_pos_string) {
		pos_string = new_pos_string;
	}
	
	void set_time_string(std::string& new_time_string) {
		time_string = new_time_string;
	}
};

using AircraftRecentEntries = RecentEntries<AircraftRecentEntry>;

class ADSBRxView : public View {
public:
	ADSBRxView(NavigationView&);
	~ADSBRxView();
	
	void focus() override;
	
	std::string title() const override { return "ADS-B receive"; };

private:
	void on_frame(const ADSBFrameMessage * message);
	
	const RecentEntriesColumns columns { {
		{ "ICAO", 6 },
		{ "Callsign", 9 },
		{ "Hits", 4 },
		{ "Time", 8 }
	} };
	AircraftRecentEntries recent { };
	RecentEntriesView<RecentEntries<AircraftRecentEntry>> recent_entries_view { columns, recent };
	
	RSSI rssi {
		{ 19 * 8, 4, 10 * 8, 8 },
	};
	
	LNAGainField field_lna {
		{ 4 * 8, 0 * 16 }
	};

	VGAGainField field_vga {
		{ 11 * 8, 0 * 16 }
	};
	
	Labels labels {
		{ { 0 * 8, 0 * 8 }, "LNA:   VGA:   RSSI:", Color::light_grey() }
	};
	
	Text text_debug_a {
		{ 0 * 8, 1 * 16, 30 * 8, 16 },
		"-"
	};
	Text text_debug_b {
		{ 0 * 8, 2 * 16, 30 * 8, 16 },
		"-"
	};
	Text text_debug_c {
		{ 0 * 8, 3 * 16, 30 * 8, 16 },
		"-"
	};
	
	MessageHandlerRegistration message_handler_frame {
		Message::ID::ADSBFrame,
		[this](Message* const p) {
			const auto message = static_cast<const ADSBFrameMessage*>(p);
			this->on_frame(message);
		}
	};
};

} /* namespace ui */
