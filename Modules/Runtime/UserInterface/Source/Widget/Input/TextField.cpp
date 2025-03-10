#include "Widget/Input/TextField.h"

namespace Ry
{

	TextField::TextField() :
		Widget()
	{
		ItemSet = MakeItemSet();
		CursorItem = MakeItem();
		SelectionItem = MakeItem();

		CursorPos = 0;
		SelectionPos = -1;
	}

	SizeType TextField::ComputeSize() const
	{
		if (bTextSizeDirty && Style)
		{
			const TextStyle& ResolvedStyle = Style->GetTextStyle(TextStyleName);
			
			CachedSize.Width = static_cast<int32>(ResolvedStyle.Font->MeasureWidth(Text));
			CachedSize.Height = static_cast<int32>(ResolvedStyle.Font->MeasureHeight(Text, static_cast<float>(CachedSize.Width)));
			bTextSizeDirty = false;
		}

		return CachedSize;
	}

	TextField& TextField::SetText(const Ry::String& Text)
	{
		this->Text = Text;
		bTextSizeDirty = true;

		if (CursorPos > Text.getSize())
		{
			CursorPos = Text.getSize();
		}

		// Pre compute text data
		ComputeTextData(ComputedTextData, Text);
		FullRefresh();

		return *this;
	}

	TextField& TextField::SetStyle(const Ry::String& TextStyleName)
	{
		this->TextStyleName = TextStyleName;
		bTextSizeDirty = true;

		FullRefresh();

		return *this;
	}

	const Ry::String& TextField::GetText() const
	{
		return Text;
	}

	void TextField::OnShow(Ry::Batch* Batch)
	{
		Widget::OnShow(Batch);

		const TextStyle& ResolvedStyle = Style->GetTextStyle(TextStyleName);

		Batch->AddItemSet(ItemSet, "Font", GetPipelineState(this), ResolvedStyle.Font->GetAtlasTexture(), WidgetLayer + 1);
		Batch->AddItem(CursorItem, "Shape", GetPipelineState(this), (Texture*) nullptr, WidgetLayer + 1);

	}

	void TextField::OnHide(Ry::Batch* Batch)
	{
		Batch->RemoveItemSet(ItemSet);
		Batch->RemoveItem(CursorItem);
		Batch->RemoveItem(SelectionItem);
	}

	void TextField::Draw()
	{
		const TextStyle& ResolvedStyle = Style->GetTextStyle(TextStyleName);

		if (IsVisible())
		{
			Point Abs = GetAbsolutePosition();
			SizeType Size = ComputeSize();
			Ry::BatchText(ItemSet, ResolvedStyle.TextColor, ResolvedStyle.Font, ComputedTextData, static_cast<float>(Abs.X), static_cast<float>(Abs.Y + Size.Height), static_cast<float>(ComputeSize().Width));

			Ry::ArrayList<float> XOffsets;
			ResolvedStyle.Font->MeasureXOffsets(XOffsets, Text);

			// Draw cursor
			float CursorX = XOffsets[CursorPos] + Abs.X;
			float Height = (float)(ResolvedStyle.Font->GetAscent() - ResolvedStyle.Font->GetDescent());
			Ry::BatchRectangle(CursorItem, WHITE, CursorX, (float)(Abs.Y + ResolvedStyle.Font->GetDescent()), 1.0f, Height, 0.0f);

			// Draw selection if applicable
			if (SelectionPos >= 0)
			{
				float CursorX = XOffsets[CursorPos];
				float SelectionX = XOffsets[SelectionPos];
				float Width = std::abs(SelectionX - CursorX);
				float X = Abs.X + (CursorX < SelectionX ? CursorX : SelectionX);

				Ry::BatchRectangle(SelectionItem, WHITE.ScaleRGB(0.1f), X, (float)(Abs.Y + ResolvedStyle.Font->GetDescent()), Width, Height, 0.0f);
			}

		}
	}

	int32 TextField::FindClosestCursorIndex(int32 Offset)
	{
		const TextStyle& ResolvedStyle = Style->GetTextStyle(TextStyleName);

		int32 RetCursorPos = -1;

		// Calc offsets
		Ry::ArrayList<float> XOffsets;
		ResolvedStyle.Font->MeasureXOffsets(XOffsets, Text);

		// Find closest offset
		int32 SmallestDiff = INT32_MAX;
		int32 Index = 0;
		while (Index < XOffsets.GetSize())
		{
			int32 Delta = (int32)(std::abs(Offset - XOffsets[Index]));
			if (Delta < SmallestDiff)
			{
				SmallestDiff = Delta;
				RetCursorPos = Index;
			}

			Index++;
		}

		return RetCursorPos;
	}

	bool TextField::OnMouseClicked(const MouseClickEvent& MouseEv)
	{
		if (MouseEv.ButtonID == 0 && MouseEv.bDoubleClick && IsHovered())
		{
			Point Abs = GetAbsolutePosition();
			int32 MouseXOffset = (int32)(MouseEv.MouseX - Abs.X);

			int32 Initial = FindClosestCursorIndex(MouseXOffset);
			CursorPos = CursorAdvanceRight(Initial);
			SelectionPos = CursorAdvanceLeft(Initial);

			UpdateSelectionBox();

			return true;
		}

		return false;
	}

	bool TextField::OnMouseDragged(const MouseDragEvent& MouseEv) 
	{
		if (MouseEv.ButtonID == 0 && bDragging)
		{
			Point Abs = GetAbsolutePosition();
			int32 MouseXOffset = (int32)(MouseEv.MouseX - Abs.X);
			CursorPos = FindClosestCursorIndex(MouseXOffset);

			UpdateSelectionBox();
			Rearrange();

		}

		return true;
	}

	bool TextField::OnMouseButtonEvent(const MouseButtonEvent& MouseEv)
	{
		if (MouseEv.ButtonID == 0)
		{
			if (MouseEv.bPressed)
			{
				if (IsHovered())
				{
					bDragging = true;

					// Find closest offset
					Point Abs = GetAbsolutePosition();
					int32 MouseXOffset = (int32)(MouseEv.MouseX - Abs.X);
					SelectionPos = FindClosestCursorIndex(MouseXOffset);
					CursorPos = SelectionPos;

				}
				else
				{
					SelectionPos = -1;
				}
			}
			else
			{
				bDragging = false;
			}
		}

		UpdateSelectionBox();
		Rearrange();

		return true;
	}

	bool TextField::IsDelimiter(char c)
	{
		if (c >= 65 && c <= 122) // alphabet
			return false;
		if (c >= 48 && c <= 57) // digits
			return false;

		return true;
	}

	bool TextField::HasSelection()
	{
		return SelectionPos >= 0 && SelectionPos != CursorPos;
	}

	void TextField::RemoveSubstring(int32 Start, int32 End)
	{
		Ry::String Prev = Text.substring(0, Start);
		Ry::String Post;
		if (End <= Text.getSize() - 1)
		{
			Post = Text.substring(End);
		}

		SetText(Prev + Post);

		if (CursorPos > Start)
		{
			CursorPos = Start;
		}

		SelectionPos = -1;
	}

	void TextField::ChopSelection()
	{
		// Modify the text
		if (SelectionPos >= 0)
		{
			int32 Start = CursorPos < SelectionPos ? CursorPos : SelectionPos;
			int32 End = CursorPos < SelectionPos ? SelectionPos : CursorPos;

			FullRefresh();
			RemoveSubstring(Start, End);
		}
	}

	void TextField::HandleBackspace()
	{
		if (HasSelection())
		{
			ChopSelection();
		}
		else if (CursorPos > 0)
		{
			RemoveSubstring(CursorPos - 1, CursorPos);
		}
	}

	void TextField::HandleLeftArrow(const KeyEvent& KeyEv)
	{
		if (KeyEv.bCtrl && KeyEv.bShift)
		{
			int32 InitialCursor = CursorPos;
			CursorPos = CursorAdvanceLeft(InitialCursor);

			if (InitialCursor != CursorPos)
			{
				if (CursorPos == SelectionPos)
				{
					// We had a selection but it was broken
					SelectionPos = -1;
				}
				else if (SelectionPos == -1 || InitialCursor == SelectionPos)
				{
					// We didn't have a selection, create one
					SelectionPos = InitialCursor;
				}
			}


		}
		else if (KeyEv.bShift)
		{
			if (SelectionPos < 0 || SelectionPos == CursorPos)
			{
				SelectionPos = CursorPos;
			}

			// Simply decrement cursor pos
			CursorPos--;
		}
		else
		{

			if (SelectionPos < 0)
			{
				CursorPos--;
			}
			else
			{
				// Left arrow takes smallest of cursor/selection pos
				CursorPos = CursorPos < SelectionPos ? CursorPos : SelectionPos;
				SelectionPos = -1;
			}
		}

		if (CursorPos < 0)
			CursorPos = 0;

		Rearrange();
		UpdateSelectionBox();
	}

	int32 TextField::CursorAdvanceLeft(int32 Initial)
	{
		int32 Increment = Initial;

		// Skip whitespace
		while (Increment > 0 && std::isspace(Text[Increment - 1]))
			Increment--;

		if (Increment > 0 && IsDelimiter(Text[Increment - 1]))
			Increment--;
		else
			// Move selection to the left (find next delimeter to the left)
			while (Increment > 0 && !IsDelimiter(Text[Increment - 1]))
				Increment--;

		Increment = std::max(Increment, 0);

		return Increment;
	}

	int32 TextField::CursorAdvanceRight(int32 Initial)
	{
		int32 Increment = Initial;
		// Skip whitespace
		while (Increment < Text.getSize() && std::isspace(Text[Increment]))
			Increment++;

		if (Increment < Text.getSize() && IsDelimiter(Text[Increment])) // Landed on a delimeter
			Increment++;
		else
			// Move selection to the right (find next delimeter to the left)
			while (Increment < Text.getSize() && !IsDelimiter(Text[Increment]))
				Increment++;

		Increment = std::min(Increment, (int32)Text.getSize());

		return Increment;
	}

	void TextField::HandleRightArrow(const KeyEvent& KeyEv)
	{
		if (KeyEv.bCtrl && KeyEv.bShift)
		{
			int32 InitialCursor = CursorPos;
			CursorPos = CursorAdvanceRight(InitialCursor);

			if (InitialCursor != CursorPos)
			{
				if (CursorPos == SelectionPos)
				{
					// We had a selection but it was broken
					SelectionPos = -1;
				}
				else if (SelectionPos == -1 || InitialCursor == SelectionPos)
				{
					// We didn't have a selection, create one
					SelectionPos = InitialCursor;
				}
			}
		}
		else if (KeyEv.bShift)
		{
			if (SelectionPos < 0 || SelectionPos == CursorPos)
			{
				SelectionPos = CursorPos;
			}

			// Simply increment cursor pos
			CursorPos++;
		}
		else
		{
			if (SelectionPos < 0)
			{
				CursorPos++;
			}
			else
			{
				// Right arrow takes biggest of cursor/selection pos
				CursorPos = CursorPos < SelectionPos ? SelectionPos : CursorPos;
				SelectionPos = -1;
			}
		}


		if (CursorPos > Text.getSize())
			CursorPos = Text.getSize();

		UpdateSelectionBox();
		Rearrange();
	}

	void TextField::InsertText(const Ry::String Insert)
	{
		if (CursorPos != SelectionPos && SelectionPos >= 0)
			ChopSelection();

		if (Text.getSize() >= 1)
		{
			Ry::String Prev = Text.substring(0, CursorPos);
			Ry::String Post;
			if (CursorPos <= Text.getSize() - 1)
			{
				Post = Text.substring(CursorPos);
			}

			SetText(Prev + Insert + Post);
		}
		else
		{
			SetText(Insert);
		}

		CursorPos += Insert.getSize();
		SelectionPos = -1; // Selection immediately goes away on type
		FullRefresh();
	}

	void TextField::UpdateSelectionBox()
	{
		if (CursorPos != SelectionPos && SelectionPos >= 0)
			GetBatch()->AddItem(SelectionItem, "Shape", GetPipelineState(this), (Texture*) nullptr, WidgetLayer);
		else
			GetBatch()->RemoveItem(SelectionItem);

		UpdateBatch();
	}

	bool TextField::OnKey(const KeyEvent& KeyEv)
	{
		if (KeyEv.Action == Ry::KeyAction::PRESS || KeyEv.Action == Ry::KeyAction::REPEAT)
		{
			if (KeyEv.bCtrl)
			{
				// Copying text
				if (KeyEv.KeyCode == KEY_C && SelectionPos >= 0 && SelectionPos != CursorPos)
				{
					int32 Start = CursorPos > SelectionPos ? SelectionPos : CursorPos;
					int32 End = CursorPos > SelectionPos ? CursorPos : SelectionPos;
					Ry::String Substring = Text.substring(Start, End);
					Ry::SetClipboardString(Substring);
				}

				// Selecting all text
				if (KeyEv.KeyCode == KEY_A)
				{
					SelectionPos = 0;
					CursorPos = Text.getSize();
				}

				// Pasting text
				if (KeyEv.KeyCode == KEY_V)
					InsertText(Ry::GetClipboardString());

			}


			if (KeyEv.KeyCode == KEY_BACKSPACE)
			{
				HandleBackspace();
			}

			if (KeyEv.KeyCode == KEY_LEFT)
			{
				HandleLeftArrow(KeyEv);
			}

			if (KeyEv.KeyCode == KEY_RIGHT)
			{
				HandleRightArrow(KeyEv);
			}

			UpdateSelectionBox();
			FullRefresh();
		}

		return true;
	}

	bool TextField::OnChar(const CharEvent& CharEv)
	{
		// Modify the text
		InsertText(Ry::String("") + static_cast<char>(CharEv.Codepoint));

		UpdateSelectionBox();
		FullRefresh();

		return true;
	}

	
}