/**
 * Copyright 2016 EMBL - European Bioinformatics Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VCF_ERROR_HPP
#define VCF_ERROR_HPP

#include <cstddef>
#include <string>
#include <stdexcept>
#include <sstream>
#include <memory>
#include <odb/core.hxx>

namespace ebi
{
  namespace vcf
  {
    enum class Severity { WARNING, ERROR };

    struct Error;
    struct MetaSectionError;
    struct HeaderSectionError;
    struct BodySectionError;
    struct NoMetaDefinitionError;
    struct FileformatError;
    struct ChromosomeBodyError;
    struct PositionBodyError;
    struct IdBodyError;
    struct ReferenceAlleleBodyError;
    struct AlternateAllelesBodyError;
    struct QualityBodyError;
    struct FilterBodyError;
    struct InfoBodyError;
    struct FormatBodyError;
    struct SamplesBodyError;
    struct SamplesFieldBodyError;
    struct NormalizationError;
    struct DuplicationError;

    /**
     *
     * This function allows throwing Error with some polymorphism, without using raw pointers.
     * This is intended to be called like ``.
     *
     * Example usage (the idiom of several `catch`es by type won't work!):
     * ```
     *  try {
     *      raise(std::make_shared<WhateverErrorChild>(params, more_params))
     *  } catch(shared_ptr<ErrorChildA> &a) {    // WRONG never will get here
     *  } catch(shared_ptr<ErrorChildB> &b) {    // WRONG never will get here
     *  } catch(shared_ptr<Error> &e) {
     *      e->apply_visitor();     // correct, will get polymorphic behaviour
     *  }
     * ```
     * @param error can be a shared pointer to a child class, but will be assigned to a shared pointer to Error.
     */
    inline void raise(std::shared_ptr<Error> error) { throw error; }

    class ErrorVisitor {
      public:
        virtual ~ErrorVisitor() {};
        virtual void visit(Error& error) = 0;
        virtual void visit(MetaSectionError &error) = 0;
        virtual void visit(HeaderSectionError &error) = 0;
        virtual void visit(BodySectionError &error) = 0;
        virtual void visit(NoMetaDefinitionError &error) = 0;
        virtual void visit(FileformatError &error) = 0;
        virtual void visit(ChromosomeBodyError &error) = 0;
        virtual void visit(PositionBodyError &error) = 0;
        virtual void visit(IdBodyError &error) = 0;
        virtual void visit(ReferenceAlleleBodyError &error) = 0;
        virtual void visit(AlternateAllelesBodyError &error) = 0;
        virtual void visit(QualityBodyError &error) = 0;
        virtual void visit(FilterBodyError &error) = 0;
        virtual void visit(InfoBodyError &error) = 0;
        virtual void visit(FormatBodyError &error) = 0;
        virtual void visit(SamplesBodyError &error) = 0;
        virtual void visit(SamplesFieldBodyError &error) = 0;
        virtual void visit(NormalizationError &error) = 0;
        virtual void visit(DuplicationError &error) = 0;
    };

    /**
     * class for VCF errors.
     *
     * Child classes may be used for more specific Errors. To add another error type, follow these steps:
     * - predeclare class before ErrorVisitor
     * - add a class at the end of this file
     * - change its name, its parent, its message and its error code
     * - add a new error code to the enum above
     * - add the new class in the get_error_instance function
     * - add a new method visit in ErrorVisitor
     */
    #pragma db object polymorphic
    struct Error : public std::runtime_error
    {
        Error() : Error{0} {}

        Error(size_t line) : Error{line, "Error, invalid file."} {}

        Error(size_t line, const std::string &message)
                : runtime_error{std::string{"Line "} + std::to_string(line) + ": " + message},
                  line{line},
                  message{message} {}

        virtual ~Error() override { }

        virtual void apply_visitor(ErrorVisitor &visitor) { visitor.visit(*this); }
        unsigned long get_id() const { return id_; }

        const size_t line;
        const std::string message;
        Severity severity;


      private:
        friend class odb::access;

        #pragma db id auto
        unsigned long id_;
    };

    #pragma db view object(Error)
    struct ErrorCount
    {
        #pragma db column("COUNT(" + Error::id_ + ")")
        std::size_t count;
    };

    // inheritance siblings depending on file location
    #pragma db object
    struct MetaSectionError : public Error
    {
        using Error::Error;
        MetaSectionError() : MetaSectionError{0} { }
        MetaSectionError(size_t line) : MetaSectionError{line, "Error in meta-data section"} { }
        virtual ~MetaSectionError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };

    #pragma db object
    struct HeaderSectionError : public Error
    {
        using Error::Error;
        HeaderSectionError() : HeaderSectionError{0} { }
        HeaderSectionError(size_t line) : HeaderSectionError{line, "Error in header section"} { }
        virtual ~HeaderSectionError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };

    #pragma db object
    struct BodySectionError : public Error
    {
        using Error::Error;
        BodySectionError() : BodySectionError{0} { }
        BodySectionError(size_t line) : BodySectionError{line, "Error in body section"} { }
        virtual ~BodySectionError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };

    #pragma db object
    struct NoMetaDefinitionError : public Error
    {
      public:
        NoMetaDefinitionError(size_t line,
                              const std::string &message,
                              const std::string &column,
                              const std::string &field)
                : Error{line, message}, column{column}, field{field} {}
        virtual ~NoMetaDefinitionError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
        std::string column;
        std::string field;
      private:
        friend class odb::access;
        NoMetaDefinitionError() {}
    };

    // inheritance siblings about detailed errors
    #pragma db object
    struct FileformatError : public MetaSectionError
    {
        using MetaSectionError::MetaSectionError;
        FileformatError() : FileformatError{0} {}
        FileformatError(size_t line) : FileformatError{line, "Error in file format section"} { }
        virtual ~FileformatError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };

    #pragma db object
    struct ChromosomeBodyError : public BodySectionError
    {
        using BodySectionError::BodySectionError;
        ChromosomeBodyError() : ChromosomeBodyError{0} {}
        ChromosomeBodyError(size_t line) : ChromosomeBodyError{line,
            "Chromosome is not a string without colons or whitespaces, optionally wrapped with angle brackets (<>)"} { }
        virtual ~ChromosomeBodyError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };

    #pragma db object
    struct PositionBodyError : public BodySectionError
    {
        using BodySectionError::BodySectionError;
        PositionBodyError() : PositionBodyError{0} {}
        PositionBodyError(size_t line) : PositionBodyError{line, "Position is not a positive number"} { }
        virtual ~PositionBodyError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };
    #pragma db object
    struct IdBodyError : public BodySectionError
    {
        using BodySectionError::BodySectionError;
        IdBodyError() : IdBodyError{0} {}
        IdBodyError(size_t line) : IdBodyError{line, "ID is not a single dot or a list of strings without semicolons or whitespaces"} { }
        virtual ~IdBodyError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };
    #pragma db object
    struct ReferenceAlleleBodyError : public BodySectionError
    {
        using BodySectionError::BodySectionError;
        ReferenceAlleleBodyError() : ReferenceAlleleBodyError{0} {}
        ReferenceAlleleBodyError(size_t line) : ReferenceAlleleBodyError{line, "Reference is not a string of bases"} { }
        virtual ~ReferenceAlleleBodyError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };
    #pragma db object
    struct AlternateAllelesBodyError : public BodySectionError
    {
        using BodySectionError::BodySectionError;
        AlternateAllelesBodyError() : AlternateAllelesBodyError{0} {}
        AlternateAllelesBodyError(size_t line) : AlternateAllelesBodyError{line, "Alternate is not a single dot or a comma-separated list of bases"} { }
        virtual ~AlternateAllelesBodyError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };
    #pragma db object
    struct QualityBodyError : public BodySectionError
    {
        using BodySectionError::BodySectionError;
        QualityBodyError() : QualityBodyError{0} {}
        QualityBodyError(size_t line) : QualityBodyError{line, "Quality is not a single dot or a positive number"} { }
        virtual ~QualityBodyError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };
    #pragma db object
    struct FilterBodyError : public BodySectionError
    {
        using BodySectionError::BodySectionError;
        FilterBodyError() : FilterBodyError{0} {}
        FilterBodyError(size_t line) : FilterBodyError{line, "Filter is not a single dot or a semicolon-separated list of strings"} { }
        virtual ~FilterBodyError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };
    #pragma db object
    struct InfoBodyError : public BodySectionError
    {
        InfoBodyError(size_t line = 0,
                      const std::string &message = "Error in info column, in body section",
                      const std::string &field = "")
                : BodySectionError{line, message}, field{field} {}
        virtual ~InfoBodyError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }

        std::string field;
    };
    #pragma db object
    struct FormatBodyError : public BodySectionError
    {
        using BodySectionError::BodySectionError;
        FormatBodyError() : FormatBodyError{0} {}
        FormatBodyError(size_t line) : FormatBodyError{line, "Format is not a colon-separated list of alphanumeric strings"} { }
        virtual ~FormatBodyError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };
    #pragma db object
    struct SamplesBodyError : public BodySectionError
    {
        using BodySectionError::BodySectionError;
        SamplesBodyError() : SamplesBodyError{0} {}
        SamplesBodyError(size_t line) : SamplesBodyError{line, "Error in samples columns, in body section"} { }
        virtual ~SamplesBodyError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };
    #pragma db object
    struct SamplesFieldBodyError : public BodySectionError
    {
      public:
        SamplesFieldBodyError(size_t line,
                              const std::string &message,
                              const std::string &field,
                              long field_cardinality = -1)
                : BodySectionError{line, message}, field{field}, field_cardinality{field_cardinality} {
            if (field.empty()) {
                throw std::invalid_argument{"SamplesFieldBodyError: field should not be an empty string. Use "
                                                    "SamplesBodyError for unknown errors in the samples columns"};
            }
        }
        virtual ~SamplesFieldBodyError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }

        std::string field;
        long field_cardinality;    // [0, inf): valid number of values. -1: unknown amount of values
      private:
        friend class odb::access;
        SamplesFieldBodyError() {}  // necessary for ODB
    };
    #pragma db object
    struct NormalizationError : public BodySectionError
    {
        using BodySectionError::BodySectionError;
        NormalizationError() : NormalizationError{0} {}
        NormalizationError(size_t line) : NormalizationError{line, "Allele normalization could not be performed"} { }
        virtual ~NormalizationError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };
    #pragma db object
    struct DuplicationError : public BodySectionError
    {
        using BodySectionError::BodySectionError;
        DuplicationError() : DuplicationError{0} {}
        DuplicationError(size_t line) : DuplicationError{line, "A duplicated variant was found"} { }
        virtual ~DuplicationError() override { }
        virtual void apply_visitor(ErrorVisitor &visitor) override { visitor.visit(*this); }
    };
  }
}

#endif //VCF_ERROR_HPP
