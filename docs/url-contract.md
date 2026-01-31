# URL Contract Specification

This document defines the URL handling rules for UncOpener. These rules form the contract between the UncClickable browser extension (which generates URLs) and UncOpener (which processes and opens them).

## Input Format

The extension produces URLs in the following format:

```
scheme://server/share/path/to/resource
```

Where:
- `scheme` is the custom scheme name configured by the user (e.g., `unc`)
- `server` is the network server name (the URL authority/host)
- `share` is the share name on the server
- `path/to/resource` is the optional path within the share

**Important**: The extension does NOT use the `scheme:server/...` format. The double-slash (`://`) is always present.

## Percent-Encoding

The UncClickable extension does NOT perform percent-encoding on URLs. Path components may contain:
- Spaces (literal space characters)
- Special characters like `#`, `?`, `&`
- Non-ASCII characters (UTF-8)

UncOpener MUST:
1. Accept URLs with or without percent-encoding
2. Decode percent-encoded sequences if present
3. Handle literal special characters correctly

## Normalization Rules

UncOpener applies the following normalization to all input URLs:

### 1. Path Separator Handling
- Forward slashes (`/`) in the URL path are converted to the appropriate format for the target platform
- Repeated slashes (`//`) are collapsed to a single separator (except for the scheme separator)

### 2. Dot Segment Handling
- Single-dot segments (`.`) are removed from the path
- Double-dot segments (`..`) are **rejected** - the URL is considered invalid
- This prevents directory traversal attacks

### 3. Trailing Slash Preservation
- A trailing slash in the input URL is preserved in the final output
- `unc://server/share/folder/` opens differently than `unc://server/share/folder`

## Validation Rules

A URL is **valid** if and only if:

1. **Scheme present**: URL starts with the configured scheme followed by `://`
2. **Authority present**: A non-empty server name follows the `://`
3. **No directory traversal**: Path contains no `..` segments
4. **UNC structure**: Path represents a valid UNC path structure (at minimum `//server/share`)

A URL is **invalid** if any of the following conditions apply:

| Condition | Example | Reason |
|-----------|---------|--------|
| Missing scheme | `//server/share` | No scheme identifier |
| Wrong scheme | `http://server/share` | Scheme mismatch |
| Missing authority | `unc:///share/path` | Empty server name |
| Single slash | `unc:/server/share` | Invalid URL format |
| Directory traversal | `unc://server/share/../other` | Security risk |
| Empty path | `unc://server` | No share specified |

## Query and Fragment Handling

Query strings (`?...`) and fragments (`#...`) in URLs are **ignored**:

- `unc://server/share/file?query=value` opens `\\server\share\file`
- `unc://server/share/file#anchor` opens `\\server\share\file`

This behavior is chosen because:
1. UNC paths do not support query strings or fragments
2. The `#` character can legitimately appear in file paths
3. Since the extension doesn't percent-encode, `#` in filenames would be ambiguous

**Note**: Literal `#` characters in path segments (before any `?` or `#` that could be interpreted as delimiters) are preserved.

## Security: UNC Allow-List

Before opening any path, UncOpener verifies the URL against the configured UNC allow-list:

1. The URL is converted to canonical UNC format: `\\server\share\path...`
2. The UNC path must start with one of the entries in the allow-list
3. Comparison is case-insensitive (Windows path semantics)

Allow-list entries:
- Use backslash separators (`\\server\share`)
- Are auto-normalized (forward slashes rejected in configuration)
- Match as prefixes (entry `\\server\share` matches `\\server\share\any\path`)

## Platform-Specific Output

### Windows

After validation, the URL is converted to a standard UNC path:

```
unc://server/share/path/to/file.txt
  -> \\server\share\path\to\file.txt
```

The path is opened using `QDesktopServices::openUrl()`.

### Linux

The URL is converted to an SMB URL:

```
unc://server/share/path/to/file.txt
  -> smb://server/share/path/to/file.txt
```

If an SMB username is configured:

```
  -> smb://username@server/share/path/to/file.txt
```

With domain-qualified username:

```
  -> smb://DOMAIN%5Cusername@server/share/path/to/file.txt
```

(The `\` in `DOMAIN\user` is percent-encoded as `%5C`)

## Test Vectors

### Valid URLs

| Input | Windows Output | Notes |
|-------|----------------|-------|
| `unc://server/share` | `\\server\share` | Minimal valid path |
| `unc://server/share/` | `\\server\share\` | Trailing slash preserved |
| `unc://server/share/path` | `\\server\share\path` | Path component |
| `unc://server/share/path/file.txt` | `\\server\share\path\file.txt` | Full path |
| `unc://server/share/path%20name` | `\\server\share\path name` | Percent-encoded space |
| `unc://server/share/path name` | `\\server\share\path name` | Literal space |
| `unc://server/share/file%23name` | `\\server\share\file#name` | Percent-encoded # |
| `unc://server/share/./file` | `\\server\share\file` | Dot segment removed |
| `unc://server/share//path` | `\\server\share\path` | Double slash collapsed |

### Invalid URLs

| Input | Rejection Reason |
|-------|------------------|
| `//server/share` | Missing scheme |
| `unc:/server/share` | Single slash (invalid format) |
| `unc:///share` | Missing authority |
| `unc://server` | Missing share |
| `unc://server/share/../other` | Directory traversal |
| `http://server/share` | Wrong scheme |
