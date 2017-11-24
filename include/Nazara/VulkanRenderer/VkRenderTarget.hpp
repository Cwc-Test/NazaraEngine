// Copyright (C) 201 Jérôme Leclercq
// This file is part of the "Nazara Engine - Vulkan Renderer"
// For conditions of distribution and use, see copyright notice in Config.hpp

#pragma once

#ifndef NAZARA_VULKANRENDERER_RENDERTARGET_HPP
#define NAZARA_VULKANRENDERER_RENDERTARGET_HPP

#include <Nazara/Prerequesites.hpp>
#include <Nazara/Core/Signal.hpp>
#include <Nazara/VulkanRenderer/Config.hpp>
#include <Nazara/VulkanRenderer/Wrapper/Framebuffer.hpp>
#include <Nazara/VulkanRenderer/Wrapper/RenderPass.hpp>
#include <Nazara/VulkanRenderer/Wrapper/Semaphore.hpp>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace Nz
{
	class NAZARA_VULKANRENDERER_API VkRenderTarget
	{
		public:
			VkRenderTarget() = default;
			VkRenderTarget(const VkRenderTarget&) = delete;
			VkRenderTarget(VkRenderTarget&&) = delete; ///TOOD?
			virtual ~VkRenderTarget();

			virtual bool Acquire(UInt32* imageIndex) const = 0;

			virtual void BuildPreRenderCommands(UInt32 imageIndex, Vk::CommandBuffer& commandBuffer) = 0;
			virtual void BuildPostRenderCommands(UInt32 imageIndex, Vk::CommandBuffer& commandBuffer) = 0;

			virtual const Vk::Framebuffer& GetFrameBuffer(UInt32 imageIndex) const = 0;
			virtual UInt32 GetFramebufferCount() const = 0;

			const Vk::RenderPass& GetRenderPass() const { return m_renderPass; }

			const Vk::Semaphore& GetRenderSemaphore() const { return m_imageReadySemaphore; }

			virtual void Present(UInt32 imageIndex) = 0;

			VkRenderTarget& operator=(const VkRenderTarget&) = delete;
			VkRenderTarget& operator=(VkRenderTarget&&) = delete; ///TOOD?

			// Signals:
			NazaraSignal(OnRenderTargetRelease,	const VkRenderTarget* /*renderTarget*/);
			NazaraSignal(OnRenderTargetSizeChange, const VkRenderTarget* /*renderTarget*/);

		protected:
			Vk::RenderPass m_renderPass;
			Vk::Semaphore m_imageReadySemaphore;
	};
}

#endif // NAZARA_VULKANRENDERER_RENDERTARGET_HPP
