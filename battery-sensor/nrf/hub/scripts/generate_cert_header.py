#!/usr/bin/env python3

import os
import sys
import argparse

def pem_to_header(pem_file, header_file, variable_name="ca_certificate"):
    """
    Convert a PEM certificate file to a C header file with the certificate 
    contents as a string literal.
    """
    try:
        with open(pem_file, 'r') as f:
            cert_content = f.read().strip()
        
        # Create header file content
        header_content = f"""#ifndef CERTIFICATES_H
#define CERTIFICATES_H

static const char {variable_name}[] = 
"""
        # Add each line of the certificate as a separate string with newline
        for line in cert_content.split('\n'):
            header_content += f'"{line}\\n"\n'
        
        # Close the string and header guard
        header_content += ';\n\n#endif /* CERTIFICATES_H */\n'
        
        # Write to header file
        with open(header_file, 'w') as f:
            f.write(header_content)
        
        print(f"Successfully generated {header_file} from {pem_file}")
        return True
    
    except Exception as e:
        print(f"Error: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description='Convert PEM certificate to C header file')
    parser.add_argument('pem_file', help='Path to input PEM certificate file')
    parser.add_argument('--output', '-o', default='../src/certificates.h',
                        help='Path to output header file (default: ../src/certificates.h)')
    parser.add_argument('--variable', '-v', default='ca_certificate',
                        help='Name of the certificate variable (default: ca_certificate)')
    
    args = parser.parse_args()
    
    return 0 if pem_to_header(args.pem_file, args.output, args.variable) else 1

if __name__ == "__main__":
    sys.exit(main())