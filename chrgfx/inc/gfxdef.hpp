#ifndef CHRGFX__GFXDEF_H
#define CHRGFX__GFXDEF_H

#include "types.hpp"
#include <png++/png.hpp>
#include <string>
#include <vector>

using png::color;
using png::palette;
using std::pair;
using std::string;
using std::vector;

namespace chrgfx
{

/**
 * Defines the bit positions of RGB channels
 * Positive shift values shift right; negative values shift left
 */
class rgb_layout
{
public:
	rgb_layout(pair<s8, u8> red, pair<s8, u8> green, pair<s8, u8> blue);

	s8 get_red_shift() const;
	u8 get_red_count() const;
	s8 get_green_shift() const;
	u8 get_green_count() const;
	s8 get_blue_shift() const;
	u8 get_blue_count() const;

private:
	pair<s8, u8> red;
	pair<s8, u8> green;
	pair<s8, u8> blue;
};

/**
 * Abtract class for a graphics definition
 */
class gfxdef
{
	/**
	 * Returns the id of this definition
	 */
public:
	string get_id() const;

protected:
	gfxdef(string const &id);

private:
	string id;
};

/**
 * Defines the pixel layout of a graphics tile by specifying the offset of
 * pixels and bitplanes
 */
class chrdef : public gfxdef
{
public:
	chrdef(string const &id, u16 const width, u16 const height,
				 u8 const bitplanes, vector<u32> const &planeoffset,
				 vector<u32> const &pixeloffset, vector<u32> const &rowoffset);

	/**
	 * Returns the width of the tile, in pixels
	 */
	u16 get_width() const;

	/**
	 * Returns the height of the tile, in pixels
	 */
	u16 get_height() const;

	/**
	 * Returns the bit depth of the tile
	 */
	u8 get_bitplanes() const;

	/**
	 * Returns the offset to a given bitplane in a row, in bits
	 */
	u32 get_planeoffset_at(size_t pos) const;

	/**
	 * Returns the offset to a given pixel in a row, in bits
	 */
	u32 get_pixeloffset_at(size_t pos) const;

	/**
	 * Returns the offset to a given row in the tile, in bits
	 */
	u32 get_rowoffset_at(size_t pos) const;

	/**
	 * Returns the size of a single tile, in bits
	 */
	u32 get_datasize() const;

private:
	u16 width;
	u16 height;
	u8 bitplanes;

	vector<u32> planeoffset;
	u32 const *planeoffset_data;

	vector<u32> pixeloffset;
	u32 const *pixeloffset_data;

	vector<u32> rowoffset;
	u32 const *rowoffset_data;

	u32 datasize; // size of one chr in bits
};

/**
 * Defines color data in one of two ways:
 *   - Specifying offsets and width of each RGB color channel within the data
 *     (rgblayout based)
 *   - Specifying a table of RGB values that approximately correspond to the
 *     output of the hardware (refpal based)
 */
class coldef : public gfxdef
{
public:
	/**
	 * Constructor for an rgblayout based coldef
	 */
	coldef(const string id, const vector<rgb_layout> layout,
				 bool is_big_endian = false);

	/**
	 * Constructor for a refpal based coldef
	 */
	coldef(const string id, const palette &refpal, bool is_big_endian = false);

	/**
	 * Returns true if this coldef is refpal based
	 */
	bool use_refpal() const;

	/**
	 * Returns the vector of RGB layouts (rgblayout based)
	 */
	vector<rgb_layout> get_rgb_layout() const;

	/**
	 * Returns the RGB bit layout for the given pass (rgblayout based)
	 */
	rgb_layout get_rgb_pass(size_t pass) const;

	/**
	 * Returns the color from the reference palette for the given index (refpal
	 * based)
	 */
	color get_refpal_entry(size_t index) const;

	/**
	 * Returns the index to the color matching the RGB value provided, or its
	 * nearest color (refpal based)
	 */
	size_t get_refpal_idx(color rgb) const;

	/**
	 * Returns true if the original harware is big endian (refpal based)
	 */
	bool get_is_big_endian() const;

private:
	vector<rgb_layout> layout;
	palette refpal;
	const bool is_refpal;

	/**
	 * Specify the endianness of the color data
	 */
	const bool is_big_endian;
};

/**
 * Defines the format of a color palette by specifying the size and count of
 * subpalettes
 */
class paldef : public gfxdef
{
public:
	paldef(string const &id, u8 const entry_datasize, u16 const subpal_length,
				 u16 const subpal_count, u8 const subpal_datasize = 0);

	u8 get_entry_datasize() const;

	u16 get_subpal_length() const;

	u16 get_subpal_count() const;

	u8 get_subpal_datasize() const;

private:
	u8 entry_datasize;
	u16 subpal_length;
	u16 subpal_count;
	u8 subpal_datasize;
};

} // namespace chrgfx

#endif
