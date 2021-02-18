/**
 * @file
 * @brief Unit tests for openshot::Cache
 * @author Jonathan Thomas <jonathan@openshot.org>
 *
 * @ref License
 */

/* LICENSE
 *
 * Copyright (c) 2008-2019 OpenShot Studios, LLC
 * <http://www.openshotstudios.com/>. This file is part of
 * OpenShot Library (libopenshot), an open-source project dedicated to
 * delivering high quality video editing and animation solutions to the
 * world. For more information visit <http://www.openshot.org/>.
 *
 * OpenShot Library (libopenshot) is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * OpenShot Library (libopenshot) is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenShot Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>
#include <QDir>

#include <catch2/catch.hpp>

#include "CacheDisk.h"
#include "CacheMemory.h"
#include "Json.h"

using namespace openshot;

TEST_CASE( "CacheMemory_Default_Constructor", "[libopenshot][cache]" )
{
	// Create cache object
	CacheMemory c;

	// Loop 50 times
	for (int i = 0; i < 50; i++)
	{
		// Add blank frame to the cache
		std::shared_ptr<Frame> f(new Frame());
		f->number = i;
		c.Add(f);
	}

	CHECK(c.Count() == 50); // Cache should have all frames, with no limit
	CHECK(c.GetMaxBytes() == 0); // Max frames should default to 0
}

TEST_CASE( "Max_Bytes_Constructor", "[libopenshot][cache]" )
{
	// Create cache object (with a max of 5 previous items)
	CacheMemory c(250 * 1024);

	// Loop 20 times
	for (int i = 30; i > 0; i--)
	{
		// Add blank frame to the cache
		std::shared_ptr<Frame> f(new Frame(i, 320, 240, "#000000"));
		f->AddColor(320, 240, "#000000");
		c.Add(f);
	}

	// Cache should have all 20
	CHECK(c.Count() == 20);

	// Add 10 frames again
	for (int i = 10; i > 0; i--)
	{
		// Add blank frame to the cache
		std::shared_ptr<Frame> f(new Frame(i, 320, 240, "#000000"));
		f->AddColor(320, 240, "#000000");
		c.Add(f);
	}

	// Count should be 20, since we're more frames than can be cached.
	CHECK(c.Count() == 20);

	// Check which items the cache kept
	CHECK(c.GetFrame(1) != nullptr);
	CHECK(c.GetFrame(10) != nullptr);
	CHECK(c.GetFrame(11) != nullptr);
	CHECK(c.GetFrame(19) != nullptr);
	CHECK(c.GetFrame(20) != nullptr);
	CHECK(c.GetFrame(21) == nullptr);
	CHECK(c.GetFrame(30) == nullptr);
}

TEST_CASE( "Clear", "[libopenshot][cache]" )
{
	// Create cache object
	CacheMemory c(250 * 1024);

	// Loop 10 times
	for (int i = 0; i < 10; i++)
	{
		// Add blank frame to the cache
		std::shared_ptr<Frame> f(new Frame());
		f->number = i;
		c.Add(f);
	}

	// Cache should only have 10 items
	CHECK(c.Count() == 10);

	// Clear Cache
	c.Clear();

	// Cache should now have 0 items
	CHECK(c.Count() == 0);
}

TEST_CASE( "Add_Duplicate_Frames", "[libopenshot][cache]" )
{
	// Create cache object
	CacheMemory c(250 * 1024);

	// Loop 10 times
	for (int i = 0; i < 10; i++)
	{
		// Add blank frame to the cache (each frame is #1)
		std::shared_ptr<Frame> f(new Frame());
		c.Add(f);
	}

	// Cache should only have 1 items (since all frames were frame #1)
	CHECK(c.Count() == 1);
}

TEST_CASE( "Check_If_Frame_Exists", "[libopenshot][cache]" )
{
	// Create cache object
	CacheMemory c(250 * 1024);

	// Loop 5 times
	for (int i = 1; i < 6; i++)
	{
		// Add blank frame to the cache
		std::shared_ptr<Frame> f(new Frame());
		f->number = i;
		c.Add(f);
	}

	// Check if certain frames exists (only 1-5 exist)
	CHECK(c.GetFrame(0) == nullptr);
	CHECK(c.GetFrame(1) != nullptr);
	CHECK(c.GetFrame(2) != nullptr);
	CHECK(c.GetFrame(3) != nullptr);
	CHECK(c.GetFrame(4) != nullptr);
	CHECK(c.GetFrame(5) != nullptr);
	CHECK(c.GetFrame(6) == nullptr);
}

TEST_CASE( "GetFrame", "[libopenshot][cache]" )
{
	// Create cache object
	CacheMemory c(250 * 1024);

	// Create 3 frames
	Frame *red = new Frame(1, 300, 300, "red");
	Frame *blue = new Frame(2, 400, 400, "blue");
	Frame *green = new Frame(3, 500, 500, "green");

	// Add frames to cache
	c.Add(std::shared_ptr<Frame>(red));
	c.Add(std::shared_ptr<Frame>(blue));
	c.Add(std::shared_ptr<Frame>(green));

	// Get frames
	CHECK(c.GetFrame(0) == nullptr);
	CHECK(c.GetFrame(4) == nullptr);

	// Check if certain frames exists (only 1-5 exist)
	CHECK(c.GetFrame(1)->number == 1);
	CHECK(c.GetFrame(2)->number == 2);
	CHECK(c.GetFrame(3)->number == 3);
}

TEST_CASE( "GetSmallest", "[libopenshot][cache]" )
{
	// Create cache object (with a max of 10 items)
	CacheMemory c(250 * 1024);

	// Create 3 frames
	Frame *red = new Frame(1, 300, 300, "red");
	Frame *blue = new Frame(2, 400, 400, "blue");
	Frame *green = new Frame(3, 500, 500, "green");

	// Add frames to cache
	c.Add(std::shared_ptr<Frame>(red));
	c.Add(std::shared_ptr<Frame>(blue));
	c.Add(std::shared_ptr<Frame>(green));

	// Check if frame 1 is the front
	CHECK(c.GetSmallestFrame()->number == 1);

	// Check if frame 1 is STILL the front
	CHECK(c.GetSmallestFrame()->number == 1);

	// Erase frame 1
	c.Remove(1);

	// Check if frame 2 is the front
	CHECK(c.GetSmallestFrame()->number == 2);
}

TEST_CASE( "Remove", "[libopenshot][cache]" )
{
	// Create cache object (with a max of 10 items)
	CacheMemory c(250 * 1024);

	// Create 3 frames
	Frame *red = new Frame(1, 300, 300, "red");
	Frame *blue = new Frame(2, 400, 400, "blue");
	Frame *green = new Frame(3, 500, 500, "green");

	// Add frames to cache
	c.Add(std::shared_ptr<Frame>(red));
	c.Add(std::shared_ptr<Frame>(blue));
	c.Add(std::shared_ptr<Frame>(green));

	// Check if count is 3
	CHECK(c.Count() == 3);

	// Check if frame 2 exists
	CHECK(c.GetFrame(2) != nullptr);

	// Remove frame 2
	c.Remove(2);

	// Check if frame 2 exists
	CHECK(c.GetFrame(2) == nullptr);

	// Check if count is 2
	CHECK(c.Count() == 2);

	// Remove frame 1
	c.Remove(1);

	// Check if frame 1 exists
	CHECK(c.GetFrame(1) == nullptr);

	// Check if count is 1
	CHECK(c.Count() == 1);
}

TEST_CASE( "Set_Max_Bytes", "[libopenshot][cache]" )
{
	// Create cache object
	CacheMemory c;

	// Loop 20 times
	for (int i = 0; i < 20; i++)
	{
		// Add blank frame to the cache
		std::shared_ptr<Frame> f(new Frame());
		f->number = i;
		c.Add(f);
	}

	CHECK(c.GetMaxBytes() == 0); // Cache defaults max frames to -1, unlimited frames

	// Set max frames
	c.SetMaxBytes(8 * 1024);
	CHECK(c.GetMaxBytes() == 8 * 1024);

	// Set max frames
	c.SetMaxBytes(4 * 1024);
	CHECK(c.GetMaxBytes() == 4 * 1024);
}

TEST_CASE( "Multiple_Remove", "[libopenshot][cache]" )
{
	// Create cache object (using platform /temp/ directory)
	CacheMemory c;

	// Add frames to disk cache
	for (int i = 1; i <= 20; i++)
	{
		// Add blank frame to the cache
		std::shared_ptr<Frame> f(new Frame());
		f->number = i;
		// Add some picture data
		f->AddColor(1280, 720, "Blue");
		f->ResizeAudio(2, 500, 44100, LAYOUT_STEREO);
		f->AddAudioSilence(500);
		c.Add(f);
	}

	// Should have 20 frames
	CHECK(c.Count() == 20);

	// Remove all 20 frames
	c.Remove(1, 20);

	// Should have 20 frames
	CHECK(c.Count() == 0);
}

TEST_CASE( "CacheDisk_Set_Max_Bytes", "[libopenshot][cache]" )
{
	QDir temp_path = QDir::tempPath() + QString("/set_max_bytes/");

	// Create cache object (using platform /temp/ directory)
	CacheDisk c(temp_path.path().toStdString(), "PPM", 1.0, 0.25);

	// Add frames to disk cache
	for (int i = 0; i < 20; i++)
	{
		// Add blank frame to the cache
		std::shared_ptr<Frame> f(new Frame());
		f->number = i;
		// Add some picture data
		f->AddColor(1280, 720, "Blue");
		f->ResizeAudio(2, 500, 44100, LAYOUT_STEREO);
		f->AddAudioSilence(500);
		c.Add(f);
	}

	CHECK(c.GetMaxBytes() == 0); // Cache defaults max frames to -1, unlimited frames

	// Set max frames
	c.SetMaxBytes(8 * 1024);
	CHECK(c.GetMaxBytes() == 8 * 1024);

	// Set max frames
	c.SetMaxBytes(4 * 1024);
	CHECK(c.GetMaxBytes() == 4 * 1024);

	// Read frames from disk cache
	std::shared_ptr<Frame> f = c.GetFrame(5);
	CHECK(f->GetWidth() == 320);
	CHECK(f->GetHeight() == 180);
	CHECK(f->GetAudioChannelsCount() == 2);
	CHECK(f->GetAudioSamplesCount() == 500);
	CHECK(f->ChannelsLayout() == LAYOUT_STEREO);
	CHECK(f->SampleRate() == 44100);

	// Check count of cache
	CHECK(c.Count() == 20);

	// Clear cache
	c.Clear();

	// Check count of cache
	CHECK(c.Count() == 0);

	// Delete cache directory
	temp_path.removeRecursively();
}

TEST_CASE( "CacheDisk_Multiple_Remove", "[libopenshot][cache]" )
{
	QDir temp_path = QDir::tempPath() + QString("/multiple_remove/");

	// Create cache object
	CacheDisk c(temp_path.path().toStdString(), "PPM", 1.0, 0.25);

	// Add frames to disk cache
	for (int i = 1; i <= 20; i++)
	{
		// Add blank frame to the cache
		std::shared_ptr<Frame> f(new Frame());
		f->number = i;
		// Add some picture data
		f->AddColor(1280, 720, "Blue");
		f->ResizeAudio(2, 500, 44100, LAYOUT_STEREO);
		f->AddAudioSilence(500);
		c.Add(f);
	}

	// Should have 20 frames
	CHECK(c.Count() == 20);

	// Remove all 20 frames
	c.Remove(1, 20);

	// Should have 20 frames
	CHECK(c.Count() == 0);

	// Delete cache directory
	temp_path.removeRecursively();
}

TEST_CASE( "CacheDisk_JSON", "[libopenshot][cache]" )
{
	QDir temp_path = QDir::tempPath() + QString("/cache_json/");

	// Create cache object (using platform /temp/ directory)
	CacheDisk c(temp_path.path().toStdString(), "PPM", 1.0, 0.25);

	// Add some frames (out of order)
	std::shared_ptr<Frame> f3(new Frame(3, 1280, 720, "Blue", 500, 2));
	c.Add(f3);
	CHECK((int)c.JsonValue()["ranges"].size() == 1);
	CHECK(c.JsonValue()["version"].asString() == "1");

	// Add some frames (out of order)
	std::shared_ptr<Frame> f1(new Frame(1, 1280, 720, "Blue", 500, 2));
	c.Add(f1);
	CHECK((int)c.JsonValue()["ranges"].size() == 2);
	CHECK(c.JsonValue()["version"].asString() == "2");

	// Add some frames (out of order)
	std::shared_ptr<Frame> f2(new Frame(2, 1280, 720, "Blue", 500, 2));
	c.Add(f2);
	CHECK((int)c.JsonValue()["ranges"].size() == 1);
	CHECK(c.JsonValue()["version"].asString() == "3");

	// Add some frames (out of order)
	std::shared_ptr<Frame> f5(new Frame(5, 1280, 720, "Blue", 500, 2));
	c.Add(f5);
	CHECK((int)c.JsonValue()["ranges"].size() == 2);
	CHECK(c.JsonValue()["version"].asString() == "4");

	// Add some frames (out of order)
	std::shared_ptr<Frame> f4(new Frame(4, 1280, 720, "Blue", 500, 2));
	c.Add(f4);
	CHECK((int)c.JsonValue()["ranges"].size() == 1);
	CHECK(c.JsonValue()["version"].asString() == "5");

	// Delete cache directory
	temp_path.removeRecursively();
}

TEST_CASE( "CacheMemory_JSON", "[libopenshot][cache]" )
{
	// Create memory cache object
	CacheMemory c;

	// Add some frames (out of order)
	std::shared_ptr<Frame> f3(new Frame(3, 1280, 720, "Blue", 500, 2));
	c.Add(f3);
	CHECK((int)c.JsonValue()["ranges"].size() == 1);
	CHECK(c.JsonValue()["version"].asString() == "1");

	// Add some frames (out of order)
	std::shared_ptr<Frame> f1(new Frame(1, 1280, 720, "Blue", 500, 2));
	c.Add(f1);
	CHECK((int)c.JsonValue()["ranges"].size() == 2);
	CHECK(c.JsonValue()["version"].asString() == "2");

	// Add some frames (out of order)
	std::shared_ptr<Frame> f2(new Frame(2, 1280, 720, "Blue", 500, 2));
	c.Add(f2);
	CHECK((int)c.JsonValue()["ranges"].size() == 1);
	CHECK(c.JsonValue()["version"].asString() == "3");

	// Add some frames (out of order)
	std::shared_ptr<Frame> f5(new Frame(5, 1280, 720, "Blue", 500, 2));
	c.Add(f5);
	CHECK((int)c.JsonValue()["ranges"].size() == 2);
	CHECK(c.JsonValue()["version"].asString() == "4");

	// Add some frames (out of order)
	std::shared_ptr<Frame> f4(new Frame(4, 1280, 720, "Blue", 500, 2));
	c.Add(f4);
	CHECK((int)c.JsonValue()["ranges"].size() == 1);
	CHECK(c.JsonValue()["version"].asString() == "5");

}
