# CYD OS — UI Design System

## Color Palette

| Token | Hex | Usage |
|-------|-----|-------|
| `COLOR_BG` | `#1A1A2E` | Main background (dark graphite) |
| `COLOR_CARD` | `#252540` | Card surfaces (charcoal) |
| `COLOR_CARD_BORDER` | `#353550` | Card borders (subtle) |
| `COLOR_STATUS_BAR` | `#121225` | Status bar background |
| `COLOR_ACCENT` | `#7FCCB0` | Primary accent (muted mint) |
| `COLOR_TEXT` | `#E8E8E8` | Primary text |
| `COLOR_TEXT_DIM` | `#888898` | Secondary/dimmed text |
| `COLOR_PASS` | `#7FCCB0` | Success/OK states |
| `COLOR_FAIL` | `#E85555` | Error/fail states |
| `COLOR_WARN` | `#E8B855` | Warning states |

## Typography

| Usage | Font | Size |
|-------|------|------|
| Title / Header | Montserrat | 16–20px |
| Body / Status | Montserrat | 12–14px |
| Caption / Status Bar | Montserrat | 10px |
| Large Display (Clock) | Montserrat | 24px |

## Component Specifications

### Status Bar
- Height: 24px
- Background: `#121225`
- Content: Left-aligned title/icon, right-aligned clock/status
- Always visible at top of screen

### Cards
- Corner radius: 8px
- Padding: 8px
- Border: 1px `#353550`
- Background: `COLOR_CARD`
- Min width: 224px (with 8px margin on each side)

### App Icons
- Size: 44×44px (icon area)
- Label below: 10px font
- Grid: 4 columns × N rows
- Padding: 8px between items

### Buttons
- Min height: 36px
- Corner radius: 6px
- Accent buttons: `COLOR_ACCENT` bg, dark text
- Ghost buttons: transparent bg, `COLOR_ACCENT` text

### Lists
- Item height: 40px
- Divider: 1px `#353550`
- Left icon: 20×20
- Right chevron or toggle

### Toggle Switch
- Width: 44px, Height: 24px
- Track: `#353550` (off), `COLOR_ACCENT` (on)
- Knob: white, 20×20

### Toast Messages
- Bottom-aligned, 8px margin
- Auto-dismiss after 2–3 seconds
- Max width: 224px
- Rounded corners: 6px

### Dialogs
- Centered overlay
- Semi-transparent backdrop
- Title + body + action buttons
- Max width: 200px

## Layout Guidelines

- Screen size: 240×320 portrait
- Status bar: 24px at top
- Usable area: 240×296
- Bottom nav: 40px (when present)
- Content area: 240×256 (with both bars)
- Side margins: 8px
- Content width: 224px
- Scrollable content for lists exceeding viewport

## States

Every data-displaying component should handle:
1. **Empty**: "No items" message with icon
2. **Loading**: Spinner with "Loading..." text
3. **Stale**: Data shown with "Updated X min ago" subtitle
4. **Error**: Error icon + message + retry button
5. **Normal**: Data displayed normally

## Animation Guidelines

- **Duration**: 80–200ms (short, restrained)
- **Easing**: Default LVGL ease-out
- **Avoid**: Continuous animation, full-screen transitions
- **Use for**: Screen transitions, toggle states, toast appear/dismiss
