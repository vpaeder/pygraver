import asyncio
from unittest.mock import Mock

__all__ = ["run_async", "AsyncMock", "AsyncListMock"]

def run_async(func):
    def wrapper(*args, **kwargs):
        asyncio.run(func(*args, **kwargs))
    return wrapper

class AsyncMock(Mock):
    async def __call__(self, *args, **kwargs):
        return super().__call__(*args, **kwargs)

class AsyncListMock(Mock):
    idx = 0
    items = []
    def __init__(self, **kwargs):
        self.items.extend(kwargs["return_value"])
        if len(self.items)==0:
            raise ValueError("return_value must be a list of items.")
        kwargs["return_value"] = self.items[0]
        super().__init__(**kwargs)

    async def __call__(self, *args, **kwargs):
        res = super().__call__(*args, **kwargs)
        self.idx = (self.idx + 1) % len(self.items)
        self.return_value = self.items[self.idx]
        return res

