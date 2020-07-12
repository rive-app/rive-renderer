#include "file.hpp"
#include "animation/animation.hpp"
#include "generated/core_context.hpp"

using namespace rive;

template <typename T = Core> static T* readRuntimeObject(BinaryReader& reader)
{
	auto coreObjectKey = reader.readVarUint();
	auto object = CoreContext::makeCoreInstance(coreObjectKey);

	while (true)
	{
		auto propertyKey = reader.readVarUint();
		if (propertyKey == 0)
		{
			// Terminator. https://media.giphy.com/media/7TtvTUMm9mp20/giphy.gif
			break;
		}
		auto propertyLength = reader.readVarUint();
		auto valueReader = reader.read(propertyLength);

		// We can get away with just checking once as our reader is safe to call
		// again after overflowing.
		if (reader.didOverflow())
		{
			delete object;
			return nullptr;
		}

		object->deserialize(propertyKey, valueReader);
	}

	// This is evaluated at compile time based on how the templated method is
	// called. This means that it'll get optimized out when calling with type
	// Core (which is the default). The type checking is skipped in this case.
	if constexpr (!std::is_same<T, Core>::value)
	{
		// Ensure the object is of the provided type, if not, return null and
		// delete the object. Note that we read in the properties regardless of
		// whether or not this object is the expected one. This ensures our
		// reader has advanced past the object.
		if (T::typeKey != object->coreType() &&
		    // It may not be that concrete type, but it does it extend that
		    // type?
		    !object->inheritsFrom(T::typeKey))
		{
			fprintf(stderr, "Expected object of type %d but found %d.\n", T::typeKey, object->coreType());
			delete object;
			return nullptr;
		}
	}
	return reinterpret_cast<T*>(object);
}

ImportResult File::import(BinaryReader& reader, File** importedFile)
{
	RuntimeHeader header;
	if (!RuntimeHeader::read(reader, header))
	{
		fprintf(stderr, "Bad header\n");
		return ImportResult::malformed;
	}
	if (header.majorVersion() != majorVersion)
	{
		fprintf(stderr, "Unsupported version %u expected %u.\n", majorVersion, header.majorVersion());
		return ImportResult::unsupportedVersion;
	}
	auto file = new File();
	auto result = file->read(reader);
	if (result != ImportResult::success)
	{
		delete file;
		return result;
	}
	*importedFile = file;
	return result;
}

ImportResult File::read(BinaryReader& reader)
{
	auto m_Backboard = readRuntimeObject<Backboard>(reader);
	if (m_Backboard == nullptr)
	{
		fprintf(stderr, "Expected first object to be the backboard.\n");
		return ImportResult::malformed;
	}

	auto numArtboards = reader.readVarUint();
	for (int i = 0; i < numArtboards; i++)
	{
		auto numObjects = reader.readVarUint();
		if (numObjects == 0)
		{
			fprintf(stderr, "Artboards must contain at least one object (themselves).\n");
			return ImportResult::malformed;
		}
		auto artboard = readRuntimeObject<Artboard>(reader);
		if (artboard == nullptr)
		{
			fprintf(stderr, "Found non-artboard in artboard list.\n");
			return ImportResult::malformed;
		}
		m_Artboards.push_back(artboard);

		artboard->addObject(artboard);

		for (int i = 1; i < numObjects; i++)
		{
			auto object = readRuntimeObject(reader);
			// N.B. we add objects that don't load (null) too as we need to look
			// them up by index.
			artboard->addObject(object);
		}

		// Animations also need to reference objects, so make sure they get read
		// in before the hierarchy resolves (batch add completes).
		auto numAnimations = reader.readVarUint();
		for (int i = 0; i < numAnimations; i++)
		{
			auto animation = readRuntimeObject<Animation>(reader);
			if (animation == nullptr)
			{
				continue;
			}
			artboard->addObject(animation);
			// animation.artboard = artboard;
		}
	}
	return ImportResult::success;
}

Backboard* File::backboard() const {return m_Backboard; }

Artboard* File::artboard(std::string name) const
{
	for (auto artboard : m_Artboards)
	{
		if (artboard->name() == name)
		{
			return artboard;
		}
	}
	return nullptr;
}

Artboard* File::artboard() const
{
	if (m_Artboards.empty())
	{
		return nullptr;
	}
	return m_Artboards[0];
}