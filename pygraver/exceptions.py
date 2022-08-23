"""Exceptions and warnings for PyGraver."""

__all__ = ["InvalidAnswerException"]

class PyGraverException(Exception):
    """Base class for custom exceptions."""
    pass

class InvalidAnswerException(PyGraverException):
    """Exception when machine answer is invalid."""
    pass