// MIT License
//
// Copyright(c) 2022-2023 Matthieu Bucchianeri
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "pch.h"

#define CHECK_OVRCMD(cmd) xr::detail::_CheckOVRResult(cmd, #cmd, FILE_AND_LINE)
#define CHECK_VKCMD(cmd) xr::detail::_CheckVKResult(cmd, #cmd, FILE_AND_LINE)

namespace xr {
    static inline std::string ToString(XrVersion version) {
        return fmt::format("{}.{}.{}", XR_VERSION_MAJOR(version), XR_VERSION_MINOR(version), XR_VERSION_PATCH(version));
    }

    static inline std::string ToString(const ovrPosef& pose) {
        return fmt::format("p: ({:.3f}, {:.3f}, {:.3f}), o:({:.3f}, {:.3f}, {:.3f}, {:.3f})",
                           pose.Position.x,
                           pose.Position.y,
                           pose.Position.z,
                           pose.Orientation.x,
                           pose.Orientation.y,
                           pose.Orientation.z,
                           pose.Orientation.w);
    }

    static inline std::string ToString(const XrPosef& pose) {
        return fmt::format("p: ({:.3f}, {:.3f}, {:.3f}), o:({:.3f}, {:.3f}, {:.3f}, {:.3f})",
                           pose.position.x,
                           pose.position.y,
                           pose.position.z,
                           pose.orientation.x,
                           pose.orientation.y,
                           pose.orientation.z,
                           pose.orientation.w);
    }

    static inline std::string ToString(const ovrVector3f& vec) {
        return fmt::format("({:.3f}, {:.3f}, {:.3f})", vec.x, vec.y, vec.z);
    }

    static inline std::string ToString(const XrVector3f& vec) {
        return fmt::format("({:.3f}, {:.3f}, {:.3f})", vec.x, vec.y, vec.z);
    }

    static inline std::string ToString(const ovrVector2f& vec) {
        return fmt::format("({:.3f}, {:.3f})", vec.x, vec.y);
    }

    static inline std::string ToString(const XrVector2f& vec) {
        return fmt::format("({:.3f}, {:.3f})", vec.x, vec.y);
    }

    static inline std::string ToString(const XrFovf& fov) {
        return fmt::format(
            "(l:{:.3f}, r:{:.3f}, u:{:.3f}, d:{:.3f})", fov.angleLeft, fov.angleRight, fov.angleUp, fov.angleDown);
    }

    static inline std::string ToString(const XrRect2Di& rect) {
        return fmt::format("x:{}, y:{} w:{} h:{}", rect.offset.x, rect.offset.y, rect.extent.width, rect.extent.height);
    }

    namespace math {

        namespace Pose {

            static inline bool Equals(const XrPosef& a, const XrPosef& b) {
                return std::abs(b.position.x - a.position.x) < 0.00001f &&
                       std::abs(b.position.y - a.position.y) < 0.00001f &&
                       std::abs(b.position.z - a.position.z) < 0.00001f &&
                       std::abs(b.orientation.x - a.orientation.x) < 0.00001f &&
                       std::abs(b.orientation.y - a.orientation.y) < 0.00001f &&
                       std::abs(b.orientation.z - a.orientation.z) < 0.00001f &&
                       std::abs(b.orientation.w - a.orientation.w) < 0.00001f;
            }

        } // namespace Pose

    } // namespace math

    namespace detail {

        [[noreturn]] static inline void _ThrowOVRResult(ovrResult ovr,
                                                        const char* originator = nullptr,
                                                        const char* sourceLocation = nullptr) {
            xr::detail::_Throw(xr::detail::_Fmt("ovrResult failure [%d]", ovr), originator, sourceLocation);
        }

        static inline HRESULT _CheckOVRResult(ovrResult ovr,
                                              const char* originator = nullptr,
                                              const char* sourceLocation = nullptr) {
            if (OVR_FAILURE(ovr)) {
                xr::detail::_ThrowOVRResult(ovr, originator, sourceLocation);
            }

            return ovr;
        }

        [[noreturn]] static inline void _ThrowVKResult(VkResult vks,
                                                       const char* originator = nullptr,
                                                       const char* sourceLocation = nullptr) {
            xr::detail::_Throw(xr::detail::_Fmt("VkStatus failure [%d]", vks), originator, sourceLocation);
        }

        static inline HRESULT _CheckVKResult(VkResult vks,
                                             const char* originator = nullptr,
                                             const char* sourceLocation = nullptr) {
            if ((vks) != VK_SUCCESS) {
                xr::detail::_ThrowVKResult(vks, originator, sourceLocation);
            }

            return vks;
        }
    } // namespace detail

} // namespace xr

namespace virtualdesktop_openxr::utils {

    // A generic timer.
    struct ITimer {
        virtual ~ITimer() = default;

        virtual void start() = 0;
        virtual void stop() = 0;

        virtual uint64_t query(bool reset = true) const = 0;
    };

    // A synchronous CPU timer.
    class CpuTimer : public ITimer {
        using clock = std::chrono::high_resolution_clock;

      public:
        void start() override {
            m_timeStart = clock::now();
        }

        void stop() override {
            m_duration += clock::now() - m_timeStart;
        }

        uint64_t query(bool reset = true) const override {
            const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(m_duration);
            if (reset)
                m_duration = clock::duration::zero();
            return duration.count();
        }

      private:
        clock::time_point m_timeStart;
        mutable clock::duration m_duration{0};
    };

    // API dispatch table for Vulkan.
    struct VulkanDispatch {
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr{nullptr};

        PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2{nullptr};
        PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties{nullptr};
        PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR{nullptr};
        PFN_vkGetDeviceQueue vkGetDeviceQueue{nullptr};
        PFN_vkQueueSubmit vkQueueSubmit{nullptr};
        PFN_vkCreateImage vkCreateImage{nullptr};
        PFN_vkDestroyImage vkDestroyImage{nullptr};
        PFN_vkAllocateMemory vkAllocateMemory{nullptr};
        PFN_vkFreeMemory vkFreeMemory{nullptr};
        PFN_vkCreateCommandPool vkCreateCommandPool{nullptr};
        PFN_vkDestroyCommandPool vkDestroyCommandPool{nullptr};
        PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers{nullptr};
        PFN_vkFreeCommandBuffers vkFreeCommandBuffers{nullptr};
        PFN_vkResetCommandBuffer vkResetCommandBuffer{nullptr};
        PFN_vkBeginCommandBuffer vkBeginCommandBuffer{nullptr};
        PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier{nullptr};
        PFN_vkCmdResetQueryPool vkCmdResetQueryPool{nullptr};
        PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp{nullptr};
        PFN_vkEndCommandBuffer vkEndCommandBuffer{nullptr};
        PFN_vkGetMemoryWin32HandlePropertiesKHR vkGetMemoryWin32HandlePropertiesKHR{nullptr};
        PFN_vkBindImageMemory vkBindImageMemory{nullptr};
        PFN_vkCreateSemaphore vkCreateSemaphore{nullptr};
        PFN_vkDestroySemaphore vkDestroySemaphore{nullptr};
        PFN_vkImportSemaphoreWin32HandleKHR vkImportSemaphoreWin32HandleKHR{nullptr};
        PFN_vkCreateFence vkCreateFence{nullptr};
        PFN_vkDestroyFence vkDestroyFence{nullptr};
        PFN_vkResetFences vkResetFences{nullptr};
        PFN_vkWaitForFences vkWaitForFences{nullptr};
        PFN_vkDeviceWaitIdle vkDeviceWaitIdle{nullptr};
        PFN_vkCreateQueryPool vkCreateQueryPool{nullptr};
        PFN_vkDestroyQueryPool vkDestroyQueryPool{nullptr};
        PFN_vkGetQueryPoolResults vkGetQueryPoolResults{nullptr};
    };

    // API dispatch table for OpenGL.
    struct GlDispatch {
        PFNGLGETUNSIGNEDBYTEVEXTPROC glGetUnsignedBytevEXT{nullptr};
        PFNGLCREATETEXTURESPROC glCreateTextures{nullptr};
        PFNGLCREATEMEMORYOBJECTSEXTPROC glCreateMemoryObjectsEXT{nullptr};
        PFNGLDELETEMEMORYOBJECTSEXTPROC glDeleteMemoryObjectsEXT{nullptr};
        PFNGLTEXTURESTORAGEMEM2DEXTPROC glTextureStorageMem2DEXT{nullptr};
        PFNGLTEXTURESTORAGEMEM2DMULTISAMPLEEXTPROC glTextureStorageMem2DMultisampleEXT{nullptr};
        PFNGLTEXTURESTORAGEMEM3DEXTPROC glTextureStorageMem3DEXT{nullptr};
        PFNGLTEXTURESTORAGEMEM3DMULTISAMPLEEXTPROC glTextureStorageMem3DMultisampleEXT{nullptr};
        PFNGLGENSEMAPHORESEXTPROC glGenSemaphoresEXT{nullptr};
        PFNGLDELETESEMAPHORESEXTPROC glDeleteSemaphoresEXT{nullptr};
        PFNGLSEMAPHOREPARAMETERUI64VEXTPROC glSemaphoreParameterui64vEXT{nullptr};
        PFNGLSIGNALSEMAPHOREEXTPROC glSignalSemaphoreEXT{nullptr};
        PFNGLIMPORTMEMORYWIN32HANDLEEXTPROC glImportMemoryWin32HandleEXT{nullptr};
        PFNGLIMPORTSEMAPHOREWIN32HANDLEEXTPROC glImportSemaphoreWin32HandleEXT{nullptr};
        PFNGLGENQUERIESPROC glGenQueries{nullptr};
        PFNGLDELETEQUERIESPROC glDeleteQueries{nullptr};
        PFNGLQUERYCOUNTERPROC glQueryCounter{nullptr};
        PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv{nullptr};
        PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v{nullptr};
    };

    struct GlContext {
        HDC glDC;
        HGLRC glRC;

        bool valid{false};
    };

    class GlContextSwitch {
      public:
        GlContextSwitch(const GlContext& context) : m_valid(context.valid) {
            if (m_valid) {
                m_glDC = wglGetCurrentDC();
                m_glRC = wglGetCurrentContext();

                wglMakeCurrent(context.glDC, context.glRC);

                // Reset error codes.
                while (glGetError() != GL_NO_ERROR)
                    ;
            }
        }

        ~GlContextSwitch() noexcept(false) {
            if (m_valid) {
                const auto error = glGetError();

                wglMakeCurrent(m_glDC, m_glRC);

                CHECK_MSG(error == GL_NO_ERROR, fmt::format("OpenGL error: 0x{:x}", error));
            }
        }

      private:
        const bool m_valid;
        HDC m_glDC;
        HGLRC m_glRC;
    };

    // https://docs.microsoft.com/en-us/archive/msdn-magazine/2017/may/c-use-modern-c-to-access-the-windows-registry
    static std::optional<int> RegGetDword(HKEY hKey, const std::string& subKey, const std::string& value) {
        DWORD data{};
        DWORD dataSize = sizeof(data);
        LONG retCode = ::RegGetValue(hKey,
                                     xr::utf8_to_wide(subKey).c_str(),
                                     xr::utf8_to_wide(value).c_str(),
                                     RRF_SUBKEY_WOW6464KEY | RRF_RT_REG_DWORD,
                                     nullptr,
                                     &data,
                                     &dataSize);
        if (retCode != ERROR_SUCCESS) {
            return {};
        }
        return data;
    }

    static std::optional<std::wstring> RegGetString(HKEY hKey, const std::string& subKey, const std::string& value) {
        DWORD dataSize = 0;
        LONG retCode = ::RegGetValue(hKey,
                                     xr::utf8_to_wide(subKey).c_str(),
                                     xr::utf8_to_wide(value).c_str(),
                                     RRF_SUBKEY_WOW6464KEY | RRF_RT_REG_SZ,
                                     nullptr,
                                     nullptr,
                                     &dataSize);
        if (retCode != ERROR_SUCCESS || !dataSize) {
            return {};
        }

        std::wstring data(dataSize / sizeof(wchar_t), 0);
        retCode = ::RegGetValue(hKey,
                                xr::utf8_to_wide(subKey).c_str(),
                                xr::utf8_to_wide(value).c_str(),
                                RRF_SUBKEY_WOW6464KEY | RRF_RT_REG_SZ,
                                nullptr,
                                data.data(),
                                &dataSize);
        if (retCode != ERROR_SUCCESS) {
            return {};
        }

        return data.substr(0, dataSize / sizeof(wchar_t) - 1);
    }

    static std::vector<const char*> ParseExtensionString(char* names) {
        std::vector<const char*> list;
        while (*names != 0) {
            list.push_back(names);
            while (*(++names) != 0) {
                if (*names == ' ') {
                    *names++ = '\0';
                    break;
                }
            }
        }
        return list;
    }

    static inline XrTime ovrTimeToXrTime(double ovrTime) {
        return (XrTime)(ovrTime * 1e9);
    }

    static inline double xrTimeToOvrTime(XrTime xrTime) {
        return xrTime / 1e9;
    }

    static inline XrPosef ovrPoseToXrPose(const ovrPosef& ovrPose) {
        XrPosef xrPose;
        xrPose.position.x = ovrPose.Position.x;
        xrPose.position.y = ovrPose.Position.y;
        xrPose.position.z = ovrPose.Position.z;
        xrPose.orientation.x = ovrPose.Orientation.x;
        xrPose.orientation.y = ovrPose.Orientation.y;
        xrPose.orientation.z = ovrPose.Orientation.z;
        xrPose.orientation.w = ovrPose.Orientation.w;

        return xrPose;
    }

    static inline ovrPosef xrPoseToOvrPose(const XrPosef& xrPose) {
        ovrPosef ovrPose;
        ovrPose.Position.x = xrPose.position.x;
        ovrPose.Position.y = xrPose.position.y;
        ovrPose.Position.z = xrPose.position.z;
        ovrPose.Orientation.x = xrPose.orientation.x;
        ovrPose.Orientation.y = xrPose.orientation.y;
        ovrPose.Orientation.z = xrPose.orientation.z;
        ovrPose.Orientation.w = xrPose.orientation.w;

        return ovrPose;
    }

    static inline XrVector3f ovrVector3dToXrVector3f(const ovrVector3f& ovrVector3f) {
        XrVector3f xrVector3f;
        xrVector3f.x = ovrVector3f.x;
        xrVector3f.y = ovrVector3f.y;
        xrVector3f.z = ovrVector3f.z;

        return xrVector3f;
    }

    static DXGI_FORMAT getTypelessFormat(DXGI_FORMAT format) {
        switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_TYPELESS;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_TYPELESS;
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
            return DXGI_FORMAT_B8G8R8X8_TYPELESS;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            return DXGI_FORMAT_R16G16B16A16_TYPELESS;
        case DXGI_FORMAT_D32_FLOAT:
            return DXGI_FORMAT_R32_TYPELESS;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            return DXGI_FORMAT_R32G8X24_TYPELESS;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            return DXGI_FORMAT_R24G8_TYPELESS;
        case DXGI_FORMAT_D16_UNORM:
            return DXGI_FORMAT_R16_TYPELESS;
        }

        return format;
    }

    static bool isSRGBFormat(DXGI_FORMAT format) {
        switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return true;
        }

        return false;
    }

    static ovrTextureFormat dxgiToOvrTextureFormat(DXGI_FORMAT format) {
        switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return OVR_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return OVR_FORMAT_B8G8R8A8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            return OVR_FORMAT_B8G8R8A8_UNORM_SRGB;
        case DXGI_FORMAT_B8G8R8X8_UNORM:
            return OVR_FORMAT_B8G8R8X8_UNORM;
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return OVR_FORMAT_B8G8R8X8_UNORM_SRGB;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            return OVR_FORMAT_R16G16B16A16_FLOAT;
        case DXGI_FORMAT_D16_UNORM:
            return OVR_FORMAT_D16_UNORM;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            return OVR_FORMAT_D24_UNORM_S8_UINT;
        case DXGI_FORMAT_D32_FLOAT:
            return OVR_FORMAT_D32_FLOAT;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            return OVR_FORMAT_D32_FLOAT_S8X24_UINT;
        default:
            return OVR_FORMAT_UNKNOWN;
        }
    }

    static DXGI_FORMAT ovrToDxgiTextureFormat(ovrTextureFormat format) {
        switch (format) {
        case OVR_FORMAT_R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case OVR_FORMAT_R8G8B8A8_UNORM_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case OVR_FORMAT_B8G8R8A8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case OVR_FORMAT_B8G8R8A8_UNORM_SRGB:
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        case OVR_FORMAT_B8G8R8X8_UNORM:
            return DXGI_FORMAT_B8G8R8X8_UNORM;
        case OVR_FORMAT_B8G8R8X8_UNORM_SRGB:
            return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
        case OVR_FORMAT_R16G16B16A16_FLOAT:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case OVR_FORMAT_D16_UNORM:
            return DXGI_FORMAT_D16_UNORM;
        case OVR_FORMAT_D24_UNORM_S8_UINT:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case OVR_FORMAT_D32_FLOAT:
            return DXGI_FORMAT_D32_FLOAT;
        case OVR_FORMAT_D32_FLOAT_S8X24_UINT:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    static ovrTextureFormat vkToOvrTextureFormat(VkFormat format) {
        switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            return OVR_FORMAT_R8G8B8A8_UNORM;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return OVR_FORMAT_B8G8R8A8_UNORM;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return OVR_FORMAT_B8G8R8A8_UNORM_SRGB;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return OVR_FORMAT_R16G16B16A16_FLOAT;
        case VK_FORMAT_D16_UNORM:
            return OVR_FORMAT_D16_UNORM;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return OVR_FORMAT_D24_UNORM_S8_UINT;
        case VK_FORMAT_D32_SFLOAT:
            return OVR_FORMAT_D32_FLOAT;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return OVR_FORMAT_D32_FLOAT_S8X24_UINT;
        default:
            return OVR_FORMAT_UNKNOWN;
        }
    }

    static ovrTextureFormat glToOvrTextureFormat(GLenum format) {
        switch (format) {
        case GL_RGBA8:
            return OVR_FORMAT_R8G8B8A8_UNORM;
        case GL_SRGB8_ALPHA8:
            return OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
        case GL_RGBA16F:
            return OVR_FORMAT_R16G16B16A16_FLOAT;
        case GL_DEPTH_COMPONENT16:
            return OVR_FORMAT_D16_UNORM;
        case GL_DEPTH24_STENCIL8:
            return OVR_FORMAT_D24_UNORM_S8_UINT;
        case GL_DEPTH_COMPONENT32F:
            return OVR_FORMAT_D32_FLOAT;
        case GL_DEPTH32F_STENCIL8:
            return OVR_FORMAT_D32_FLOAT_S8X24_UINT;
        default:
            return OVR_FORMAT_UNKNOWN;
        }
    }

    static size_t glGetBytePerPixels(GLenum format) {
        switch (format) {
        case GL_DEPTH_COMPONENT16:
            return 2;
        case GL_RGBA8:
        case GL_SRGB8_ALPHA8:
        case GL_DEPTH24_STENCIL8:
        case GL_DEPTH_COMPONENT32F:
        case GL_R11F_G11F_B10F:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            return 4;
        case GL_RGBA16F:
        case GL_DEPTH32F_STENCIL8:
            return 8;
        default:
            return 0;
        }
    }

    static inline bool isValidSwapchainRect(ovrTextureSwapChainDesc desc, XrRect2Di rect) {
        if (rect.offset.x < 0 || rect.offset.y < 0 || rect.extent.width <= 0 || rect.extent.height <= 0) {
            return false;
        }

        if (rect.offset.x + rect.extent.width > desc.Width || rect.offset.y + rect.extent.height > desc.Height) {
            return false;
        }

        return true;
    }

    static inline void setDebugName(ID3D11DeviceChild* resource, std::string_view name) {
        if (resource && !name.empty()) {
            resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.data());
        }
    }

    static inline void setDebugName(ID3D12Object* resource, std::string_view name) {
        if (resource && !name.empty()) {
            resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.data());
        }
    }

    static inline bool startsWith(const std::string& str, const std::string& substr) {
        return str.find(substr) == 0;
    }

    static inline bool endsWith(const std::string& str, const std::string& substr) {
        const auto pos = str.find(substr);
        return pos != std::string::npos && pos == str.size() - substr.size();
    }

} // namespace virtualdesktop_openxr::utils

#include "gpu_timers.h"
