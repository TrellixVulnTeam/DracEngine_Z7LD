#pragma once

#include "Widget/Layout/PanelWidget.h"
#include "Drawable.h"
#include "ScrollPane.gen.h"

namespace Ry
{

	constexpr uint8 SCROLL_METHOD_TOP_TO_BOTTOM = 0x0000;
	constexpr uint8 SCROLL_METHOD_BOTTOM_TO_TOP = 0x0001;

	class USERINTERFACE_MODULE ScrollPane : public PanelWidget
	{
	public:

		GeneratedBody()

		RefField()
		uint8 VerticalPlacement;

		struct Slot : public PanelWidgetSlot
		{
			Slot() :
				PanelWidgetSlot()
			{
			}

			Slot(SharedPtr<Ry::Widget> Wid):
				PanelWidgetSlot(Wid)
			{
			}

			ScrollPane::Slot& operator[](SharedPtr<Ry::Widget> Child)
			{
				this->Widget = Child;
				return *this;
			}

		};

		ScrollPane();
		float GetVerticalScrollAmount();
		float GetHorizontalScrollAmount();
		void VerticalScroll(float Pixels);
		void HorizontalScroll(float Pixels);
		void SetHorizontalScrollAmount(float Scroll);
		void SetVerticalScrollAmount(float Scroll);
		static ScrollPane::Slot MakeSlot();
		SharedPtr<PanelWidgetSlot> AppendSlot(SharedPtr<Widget> Widget) override;
		void Draw() override;
		/**
		 * Arrange widgets vertically.
		 */
		void Arrange() override;
		PipelineState GetPipelineState(const Widget* ForWidget) const override;
		void GetPipelineStates(Ry::ArrayList<PipelineState>& OutStates, bool bRecurse) override;

		RectScissor GetClipSpace(const Widget* ForWidget) const override;
		void OnShow(Ry::Batch* Batch) override;
		void OnHide(Ry::Batch* Batch) override;
		void Update() override;
		SizeType ComputeSize() const override;
		void ClearChildren() override;
		bool OnMouseScroll(const MouseScrollEvent& MouseEv) override;
		bool OnMouseButtonEvent(const MouseButtonEvent& MouseEv) override;
		bool OnMouseEvent(const MouseEvent& MouseEv) override;

	private:

		mutable SizeType SizeCache;

		bool ShouldShowVertScrollbar();
		bool ShouldShowHorizontalScrollbar();

		void GetVertScrollBarBounds(Point& OutPos, SizeType& OutSize);
		void GetHorScrollBarBounds(Point& OutPos, SizeType& OutSize);
		// Utility to compute the size of the children embedded within the scroll pane
		SizeType ComputeChildrenSize() const;

		Ry::ArrayList<SharedPtr<Slot>> ChildrenSlots;

		Ry::SharedPtr<BatchItem> DebugRect;

		float VerticalScrollAmount = 1.0f;
		float HorizontalScrollAmount;

		float ScrollBarThickness;

		Ry::SharedPtr<BatchItemSet> HorizontalBarItem;
		Ry::SharedPtr<BatchItemSet> VerticalBarItem;

		Ry::SharedPtr<BoxDrawable> VerticalScrollBar;
		Ry::SharedPtr<BoxDrawable> HorizontalScrollBar;

		bool bVerticalBarPressed;
		bool bHorizontalBarPressed;
		Point RelativeScrollBarPressed;

		bool bAllowHorizontalScroll;

		RectScissor LastClip;
	} RefClass();

}
