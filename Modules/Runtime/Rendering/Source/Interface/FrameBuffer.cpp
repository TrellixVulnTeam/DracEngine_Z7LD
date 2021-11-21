#include "Interface/FrameBuffer.h"

namespace Ry
{
	
	FrameBuffer::FrameBuffer(uint32 Width, uint32 Height, int32 Samples):
	IntendedWidth(Width), IntendedHeight(Height), Samples(Samples)
	{
		// Null out the default texture attachments
		for (int32 Index = 0; Index < MAX_COLOR_ATTACHMENTS2; Index++)
		{
			ColorAttachments[Index] = nullptr;
		}

		DepthAttachment = nullptr;
		StencilAttachment = nullptr;

		this->ColorAttachmentCount = 0;
	};
	
}