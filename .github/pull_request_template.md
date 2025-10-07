# ✅ Pull Request Template – `vixcpp/json`

## 📄 Description

Please provide a clear and concise summary of the changes in this PR,  
along with the motivation behind them (e.g., performance improvements, new feature, code cleanup, etc.).

Reference any related issues or feature requests:  
**Fixes** # (issue number)

---

## 🔍 Type of Change

Please check all that apply:

- [ ] 🐞 **Bug fix** (non-breaking change that fixes an issue)
- [ ] ✨ **New feature** (non-breaking change that adds functionality)
- [ ] 💥 **Breaking change** (modifies existing functionality or API)
- [ ] 📝 **Documentation update**
- [ ] ✅ **Tests / CI improvements**

---

## 🧪 How Has This Been Tested?

Briefly describe how you verified your changes. Include exact steps if others want to reproduce:

```bash
# Create and enter build directory
mkdir build && cd build

# Configure the project (example builds are enabled by default)
cmake ..

# Build all targets
make -j$(nproc)
```

You should see example binaries compiled, such as:

vix_json_quick

vix_json_builders

vix_json_jpath

vix_json_io

You can run them individually:

```bash
./vix_json_quick
./vix_json_builders
./vix_json_jpath
./vix_json_io
```

If applicable, mention unit or integration tests you added.

✅ Checklist

My code follows the style guidelines of this project

I have reviewed my own code

I have commented my code where needed

I have updated the relevant documentation

I have added tests that prove my fix is effective or my feature works

All new and existing tests pass

🧩 Additional Notes

Add any other information or context here (e.g., screenshots, benchmarks, known limitations, performance graphs, etc.).
