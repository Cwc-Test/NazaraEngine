// Copyright (C) 2016 Jérôme Leclercq
// This file is part of the "Nazara Engine - Vulkan"
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NAZARA_VULKAN_HPP
#define NAZARA_VULKAN_HPP

#include <Nazara/Prerequesites.hpp>
#include <Nazara/Core/Initializer.hpp>
#include <Nazara/Core/ParameterList.hpp>
#include <Nazara/Vulkan/Config.hpp>
#include <Nazara/Vulkan/VkInstance.hpp>

namespace Nz
{
	class NAZARA_VULKAN_API Vulkan
	{
		public:
			Vulkan() = delete;
			~Vulkan() = delete;

			static Vk::Instance& GetInstance();

			static bool Initialize();

			static bool IsInitialized();

			static void SetParameters(const ParameterList& parameters);

			static void Uninitialize();

		private:
			static Vk::Instance s_instance;
			static ParameterList s_initializationParameters;
			static unsigned int s_moduleReferenceCounter;
	};	
}

#endif // NAZARA_VULKAN_HPP